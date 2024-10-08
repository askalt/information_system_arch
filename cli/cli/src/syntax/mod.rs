use std::collections::VecDeque;

use crate::cmd::{
    cat::CatCmd, echo::EchoCmd, exit::ExitCmd, noop::NoopCmd, proc::ProcCmd, pwd::PwdCmd,
    wc::WcCmd, Cmd, EnvAssign,
};

#[derive(PartialEq, Debug, Clone)]
enum Token {
    Eps,
    Path { src: Vec<u8>, is_lvalue: bool },
    Pipe,
    SetCmd { lvalue: Vec<u8>, rvalue: Vec<u8> },
}

impl Token {
    fn is_ident_char(b: u8) -> bool {
        b.is_ascii_alphabetic()
    }

    fn feed(self, b: u8, expect_path: bool) -> Token {
        match self {
            Token::Eps => Token::Path {
                src: vec![b],
                is_lvalue: Self::is_ident_char(b),
            },
            Token::Path { mut src, is_lvalue } => {
                if b == b'=' && is_lvalue && !expect_path {
                    Token::SetCmd {
                        lvalue: src,
                        rvalue: vec![],
                    }
                } else {
                    src.push(b);
                    Token::Path {
                        src: src,
                        is_lvalue: is_lvalue && Self::is_ident_char(b),
                    }
                }
            }
            Token::Pipe => unreachable!(),
            Token::SetCmd { lvalue, mut rvalue } => {
                rvalue.push(b);
                Token::SetCmd {
                    lvalue: lvalue,
                    rvalue: rvalue,
                }
            }
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

struct Lexer {
    state: LexerState,
    pos: usize,
    buf: Vec<u8>,
    deffered: VecDeque<Token>,
}

impl Lexer {
    fn new() -> Self {
        Self {
            state: Default::default(),
            pos: 0,
            buf: vec![],
            deffered: VecDeque::new(),
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
                    self.state.token = self.state.token.feed(b, expect_path);
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
                                        }
                                    }
                                    /* Nothing changes for other states. */
                                    Token::SetCmd { .. } | Token::Path { .. } => {}
                                    Token::Pipe => unreachable!(),
                                }
                            } else {
                                self.state.token = self.state.token.feed(b, expect_path);
                            }
                        }
                        None => match b {
                            b' ' => match self.state.token {
                                Token::Eps => {
                                    continue;
                                }
                                Token::SetCmd { .. } | Token::Path { .. } => {
                                    return (Some(self.state.token.clone()), self);
                                }
                                Token::Pipe => unreachable!(),
                            },
                            b'\\' => {
                                self.state.escape_next = true;
                            }
                            b'\'' | b'\"' => {
                                self.state.quote = Some(b);
                            }
                            b'|' => match self.state.token {
                                Token::Eps => return (Some(Token::Pipe), self),
                                Token::SetCmd { .. } | Token::Path { .. } => {
                                    self.deffered.push_back(Token::Pipe);
                                    return (Some(self.state.token.clone()), self);
                                }
                                Token::Pipe => unreachable!(),
                            },
                            _ => {
                                self.state.token = self.state.token.feed(b, expect_path);
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

pub struct Parser {
    lexer: Lexer,
    state: ParserState,
    cmds: Vec<Box<dyn Cmd>>,
}

impl Parser {
    pub fn new() -> Self {
        Self {
            lexer: Lexer::new(),
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
                        Token::SetCmd { lvalue, rvalue } => {
                            self.state = ParserState::Assigns {
                                assigns: vec![EnvAssign::new(lvalue, rvalue)],
                            };
                        }
                    },
                    /* Assigns. */
                    ParserState::Assigns { mut assigns } => match token {
                        Token::Eps => {
                            return (Some(Ok(self.cmds)), None);
                        }
                        Token::Pipe => {
                            self.cmds.push(Box::new(NoopCmd::new()));
                            self.state = ParserState::Init { piped: false };
                        }
                        Token::Path { src, .. } => {
                            self.state = ParserState::Args {
                                assigns: assigns,
                                cmd_path: src,
                                args: vec![],
                            };
                        }
                        Token::SetCmd { lvalue, rvalue } => {
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
    use crate::syntax::Token;

    use super::Lexer;

    #[test]
    fn test_lexer_simple() {
        let line = b"    awda75 awdwe llllplk  ".to_vec();
        let lexer = Lexer::new();

        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
        lexer.feed(line);

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awda75".to_vec(),
                is_lvalue: false
            })
        );

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awdwe".to_vec(),
                is_lvalue: true
            })
        );

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"llllplk".to_vec(),
                is_lvalue: true
            })
        );

        let (actual, _) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
    }

    #[test]
    fn test_lexer_expect_next() {
        let mut line = b"    awda75 \"wdwdw  ".to_vec();
        let lexer = Lexer::new();

        let (actual, mut lexer) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
        lexer.feed(line);

        let (actual, lexer) = lexer.next(true);
        assert_eq!(
            actual,
            Some(Token::Path {
                src: b"awda75".to_vec(),
                is_lvalue: false
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
                is_lvalue: false
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
                is_lvalue: false
            })
        );
        let (actual, _) = lexer.next(true);
        assert_eq!(actual, Some(Token::Eps));
    }
}
