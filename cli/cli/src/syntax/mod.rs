use std::collections::VecDeque;

use crate::cmd::{
    cat::CatCmd, echo::EchoCmd, env::Env, exit::ExitCmd, assign::AssignCmd, proc::ProcCmd, pwd::PwdCmd,
    wc::WcCmd, Cmd, EnvAssign,
};

const SUB_SIGN: u8 = b'$';

#[derive(PartialEq, Debug, Clone)]
enum Token {
    Eps,
    Path {
        src: Vec<u8>,
        sub: Option<Vec<u8>>,
        is_lvalue: bool,
    },
    Pipe,
    SetCmd {
        lvalue: Vec<u8>,
        rvalue: Vec<u8>,
        sub: Option<Vec<u8>>,
    },
}

impl Token {
    fn is_ident_char(b: u8) -> bool {
        b.is_ascii_alphabetic()
    }

    fn feed(self, b: u8, expect_path: bool, env: &Env) -> Token {
        match self {
            Token::Eps => match b {
                SUB_SIGN => Token::Path {
                    src: vec![],
                    sub: Some(vec![]),
                    is_lvalue: false,
                },
                b => Token::Path {
                    src: vec![b],
                    sub: None,
                    is_lvalue: Self::is_ident_char(b),
                },
            },
            Token::Path {
                mut src,
                sub,
                is_lvalue,
            } => match sub {
                Some(mut sub) => {
                    if Self::is_ident_char(b) {
                        sub.push(b);
                        Token::Path {
                            src: src,
                            sub: Some(sub),
                            is_lvalue: is_lvalue,
                        }
                    } else {
                        /* Perform substitution. */
                        let mut sub = env.get(sub);
                        src.append(&mut sub);
                        assert!(!is_lvalue);
                        if b != SUB_SIGN {
                            src.push(b);
                        }
                        Token::Path {
                            src: src,
                            sub: if b == SUB_SIGN { Some(vec![]) } else { None },
                            is_lvalue: is_lvalue,
                        }
                    }
                }
                None => {
                    if b == b'=' && is_lvalue && !expect_path {
                        Token::SetCmd {
                            lvalue: src,
                            rvalue: vec![],
                            sub: None,
                        }
                    } else if b == SUB_SIGN {
                        Token::Path {
                            src: src,
                            sub: Some(vec![]),
                            is_lvalue: false,
                        }
                    } else {
                        src.push(b);
                        Token::Path {
                            src: src,
                            sub: None,
                            is_lvalue: is_lvalue && Self::is_ident_char(b),
                        }
                    }
                }
            },
            Token::SetCmd {
                lvalue,
                mut rvalue,
                sub,
            } => match sub {
                Some(mut sub) => {
                    if Self::is_ident_char(b) {
                        sub.push(b);
                        Token::SetCmd {
                            lvalue: lvalue,
                            rvalue: rvalue,
                            sub: Some(sub),
                        }
                    } else {
                        /* Perform substitution. */
                        let mut sub = env.get(sub);
                        rvalue.append(&mut sub);
                        if b != SUB_SIGN {
                            /* a=$b* */
                            rvalue.push(b);
                        }
                        Token::SetCmd {
                            lvalue: lvalue,
                            rvalue: rvalue,
                            sub: if b == SUB_SIGN { Some(vec![]) } else { None },
                        }
                    }
                }
                None => {
                    rvalue.push(b);
                    Token::SetCmd {
                        lvalue: lvalue,
                        rvalue: rvalue,
                        sub: None,
                    }
                }
            },
            Token::Pipe => unreachable!(),
        }
    }

    fn finalize(self, env: &Env) -> Self {
        match self {
            Token::Eps => Token::Eps,
            Token::Path {
                mut src,
                sub,
                is_lvalue,
            } => match sub {
                Some(sub) => {
                    /* Perform substitution. */
                    let mut sub = env.get(sub);
                    src.append(&mut sub);
                    assert!(!is_lvalue);
                    Token::Path {
                        src: src,
                        sub: None,
                        is_lvalue: is_lvalue,
                    }
                }
                None => Token::Path {
                    src: src,
                    sub: sub,
                    is_lvalue: is_lvalue,
                },
            },
            Token::SetCmd {
                lvalue,
                mut rvalue,
                sub,
            } => match sub {
                Some(sub) => {
                    /* Perform substitution. */
                    let mut sub = env.get(sub);
                    rvalue.append(&mut sub);
                    Token::SetCmd {
                        lvalue: lvalue,
                        rvalue: rvalue,
                        sub: None,
                    }
                }
                None => Token::SetCmd {
                    lvalue: lvalue,
                    rvalue: rvalue,
                    sub: sub,
                },
            },
            Token::Pipe => unreachable!(),
        }
    }
}

struct LexerState {
    escape_next: bool,
    quote: Option<u8>,
    token: Token,
}

impl Default for LexerState {
    fn default() -> Self {
        Self {
            escape_next: false,
            quote: None,
            token: Token::Eps,
        }
    }
}

struct Lexer<'a> {
    state: LexerState,
    pos: usize,
    buf: Vec<u8>,
    deffered: VecDeque<Token>,
    env: &'a Env,
}

impl<'a> Lexer<'a> {
    fn new(env: &'a Env) -> Self {
        Self {
            state: Default::default(),
            pos: 0,
            buf: vec![],
            deffered: VecDeque::new(),
            env: env,
        }
    }

    fn feed(&mut self, buf: Vec<u8>) {
        self.pos = 0;
        self.buf = buf;
    }

    fn next(mut self, expect_path: bool) -> (Option<Token>, Self) {
        if !self.deffered.is_empty() {
            return (self.deffered.pop_front(), self);
        }
        if !(self.state.escape_next || self.state.quote.is_some()) {
            /* We don't expect continuation, so start from the beginning. */
            self.state = Default::default();
        }

        while self.pos < self.buf.len() {
            let b = self.buf[self.pos];
            self.pos += 1;
            match self.state.escape_next {
                true => {
                    self.state.token = self.state.token.feed(b, expect_path, self.env);
                }
                false => {
                    match self.state.quote {
                        Some(quote) => {
                            if b == quote {
                                self.state.quote = None;
                                match self.state.token {
                                    Token::Eps => {
                                        self.state.token = Token::Path {
                                            src: vec![],
                                            is_lvalue: false,
                                            sub: None,
                                        }
                                    }
                                    /* Nothing changes for other states. */
                                    Token::SetCmd { .. } | Token::Path { .. } => {}
                                    Token::Pipe => unreachable!(),
                                }
                            } else {
                                self.state.token = self.state.token.feed(b, expect_path, self.env);
                            }
                        }
                        None => match b {
                            b' ' => {
                                self.state.token = self.state.token.finalize(self.env);
                                match self.state.token {
                                    Token::Eps => {
                                        continue;
                                    }
                                    Token::SetCmd { .. } | Token::Path { .. } => {
                                        return (Some(self.state.token.clone()), self);
                                    }
                                    Token::Pipe => unreachable!(),
                                }
                            }
                            b'\\' => {
                                self.state.escape_next = true;
                            }
                            b'\'' | b'\"' => {
                                self.state.quote = Some(b);
                            }
                            b'|' => {
                                self.state.token = self.state.token.finalize(&self.env);
                                match self.state.token {
                                    Token::Eps => return (Some(Token::Pipe), self),
                                    Token::SetCmd { .. } | Token::Path { .. } => {
                                        self.deffered.push_back(Token::Pipe);
                                        return (Some(self.state.token.clone()), self);
                                    }
                                    Token::Pipe => unreachable!(),
                                }
                            }
                            _ => {
                                self.state.token = self.state.token.feed(b, expect_path, self.env);
                            }
                        },
                    }
                }
            }
        }

        (
            if self.state.escape_next || self.state.quote.is_some() {
                /* Wait for another characters. */
                None
            } else {
                self.state.token = self.state.token.finalize(&self.env);
                Some(self.state.token.clone())
            },
            self,
        )
    }
}

enum ParserState {
    Init {
        piped: bool,
    },
    /* Assigns are accumulated assigns. */
    Assigns {
        assigns: Vec<EnvAssign>,
    },
    Args {
        assigns: Vec<EnvAssign>,
        cmd_path: Vec<u8>,
        args: Vec<Vec<u8>>,
    },
}

pub struct Parser<'a> {
    lexer: Lexer<'a>,
    state: ParserState,
    cmds: Vec<Box<dyn Cmd>>,
}

impl<'a> Parser<'a> {
    pub fn new(env: &'a Env) -> Self {
        Self {
            lexer: Lexer::new(env),
            state: ParserState::Init { piped: false },
            cmds: vec![],
        }
    }

    fn create_command(
        assigns: &Vec<EnvAssign>,
        path: &Vec<u8>,
        args: &Vec<Vec<u8>>,
    ) -> Box<dyn Cmd> {
        let args = args.clone();
        match path.as_slice() {
            b"cat" => Box::new(CatCmd::new(args)),
            b"echo" => Box::new(EchoCmd::new(args)),
            b"exit" => Box::new(ExitCmd::new(args)),
            b"wc" => Box::new(WcCmd::new(args)),
            b"pwd" => Box::new(PwdCmd::new(args)),
            p => Box::new(ProcCmd::new(p.to_vec(), args, assigns.clone())),
        }
    }

    pub fn feed(
        mut self,
        buf: Vec<u8>,
    ) -> (Option<anyhow::Result<Vec<Box<dyn Cmd>>>>, Option<Self>) {
        /* Provide new buffer for lexer. */
        self.lexer.feed(buf);

        loop {
            let expect_path = matches!(self.state, ParserState::Args { .. });
            let (token, lexer) = self.lexer.next(expect_path);
            self.lexer = lexer;
            match token {
                None => return (None, Some(self)),
                Some(token) => match self.state {
                    /* Init. */
                    ParserState::Init { piped } => match token {
                        Token::Eps => {
                            return {
                                if piped {
                                    (
                                        Some(Err(anyhow::anyhow!(
                                            "syntax error near unexpected token `|'"
                                        ))),
                                        None,
                                    )
                                } else {
                                    (Some(Ok(self.cmds)), None)
                                }
                            }
                        }
                        Token::Pipe => {
                            if piped {
                                return (
                                    Some(Err(anyhow::anyhow!(
                                        "syntax error near unexpected token `|'"
                                    ))),
                                    None,
                                );
                            } else {
                                self.state = ParserState::Init { piped: true };
                            }
                        }
                        Token::Path { src, .. } => {
                            self.state = ParserState::Args {
                                assigns: vec![],
                                cmd_path: src,
                                args: vec![],
                            };
                        }
                        Token::SetCmd { lvalue, rvalue, .. } => {
                            self.state = ParserState::Assigns {
                                assigns: vec![EnvAssign::new(lvalue, rvalue)],
                            };
                        }
                    },
                    /* Assigns. */
                    ParserState::Assigns { mut assigns } => match token {
                        Token::Eps => {
                            if self.cmds.is_empty() {
                                /* Only when we do assigns for global env. */
                                self.cmds.push(Box::new(AssignCmd::new(assigns)));
                            }
                            return (Some(Ok(self.cmds)), None);
                        }
                        Token::Pipe => {
                            self.cmds.push(Box::new(AssignCmd::new(vec![])));
                            self.state = ParserState::Init { piped: false };
                        }
                        Token::Path { src, .. } => {
                            self.state = ParserState::Args {
                                assigns: assigns,
                                cmd_path: src,
                                args: vec![],
                            };
                        }
                        Token::SetCmd { lvalue, rvalue, .. } => {
                            assigns.push(EnvAssign::new(lvalue, rvalue));
                            self.state = ParserState::Assigns { assigns: assigns };
                        }
                    },
                    /* Args */
                    ParserState::Args {
                        assigns,
                        cmd_path,
                        mut args,
                    } => match token {
                        Token::Eps => {
                            let mut cmds = self.cmds;
                            cmds.push(Self::create_command(&assigns, &cmd_path, &args));
                            return (Some(Ok(cmds)), None);
                        }
                        Token::Pipe => {
                            self.cmds
                                .push(Self::create_command(&assigns, &cmd_path, &args));
                            self.state = ParserState::Init { piped: true };
                        }
                        Token::Path { src, .. } => {
                            args.push(src);
                            self.state = ParserState::Args {
                                assigns: assigns,
                                cmd_path: cmd_path,
                                args: args,
                            };
                        }
                        Token::SetCmd { .. } => {
                            unreachable!("`expect_path` = true, so we don't expect set cmd ")
                        }
                    },
                },
            };
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{cmd::env::Env, syntax::Token};

    use super::Lexer;

    #[test]
    fn test_lexer_simple() {
        let line = b"    awda75 awdwe llllplk  ".to_vec();
        let env = Env::new();
        let lexer = Lexer::new(&env);

        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
        lexer.feed(line);

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awda75".to_vec(),
                is_lvalue: false,
                sub: None,
            })
        );

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awdwe".to_vec(),
                is_lvalue: true,
                sub: None,
            })
        );

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"llllplk".to_vec(),
                is_lvalue: true,
                sub: None,
            })
        );

        let (actual, _) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
    }

    #[test]
    fn test_lexer_expect_next() {
        let mut line = b"    awda75 \"wdwdw  ".to_vec();
        let env = Env::new();
        let lexer = Lexer::new(&env);

        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
        lexer.feed(line);

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awda75".to_vec(),
                is_lvalue: false,
                sub: None,
            })
        );
        let (actual, lexer) = lexer.next(true);
        assert_eq!(actual, None);
        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, None);

        line = b" awdwd\"".to_vec();
        lexer.feed(line);
        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"wdwdw   awdwd".to_vec(),
                is_lvalue: false,
                sub: None,
            })
        );
        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));

        line = b" \'wdwdw  ".to_vec();
        lexer.feed(line);
        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, None);

        line = b" awdwd\'".to_vec();
        lexer.feed(line);
        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"wdwdw   awdwd".to_vec(),
                is_lvalue: false,
                sub: None,
            })
        );
        let (actual, _) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
    }
}
