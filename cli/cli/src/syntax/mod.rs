use std::collections::VecDeque;

use crate::cmd::{
    cat::CatCmd, echo::EchoCmd, exit::ExitCmd, proc::ProcCmd, pwd::PwdCmd, setenv::SetEnvCmd,
    wc::WcCmd, Cmd,
};

#[derive(PartialEq, Debug)]
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

struct Lexer {
    state: LexerState,
    pos: usize,
    buf: Vec<u8>,
    deffered: VecDeque<Token>,
}

impl Lexer {
    fn new() -> Self {
        Self {
            state: LexerState {
                escape_next: false,
                quote: None,
                token: Token::Eps,
            },
            pos: 0,
            buf: vec![],
            deffered: VecDeque::new(),
        }
    }

    fn feed(&mut self, buf: Vec<u8>) {
        self.pos = 0;
        self.buf = buf;
    }

    fn push(&mut self, b: u8, expect_path: bool) {
        self.state.token = self.state.token.feed(b, expect_path)
    }

    fn next(&mut self, expect_path: bool) -> Option<Token> {
        if !self.deffered.is_empty() {
            return self.deffered.pop_front();
        }
        if !(self.state.escape_next || self.state.quote.is_some()) {
            self.acc = None;
        }
        while self.pos < self.buf.len() {
            let b = self.buf[self.pos];
            self.pos += 1;
            if self.state.escape_next {
                self.push(b);
                self.state.escape_next = false;
            } else {
                if b == b'\\' {
                    self.state.escape_next = true;
                } else if let Some(quote) = self.state.quote {
                    if b == quote {
                        self.state.quote = None;
                        if self.acc == None {
                            self.acc = Some(vec![])
                        }
                    } else {
                        self.push(b);
                    }
                } else {
                    match b {
                        b' ' => {
                            if let Some(ref acc) = self.acc {
                                return Some(Token::Path(acc.clone()));
                            }
                        }
                        b'\'' | b'\"' => {
                            self.state.quote = Some(b);
                        }
                        b'|' => {
                            if let Some(ref acc) = self.acc {
                                self.deffered.push_back(Token::Pipe);
                                return Some(Token::Path(acc.clone()));
                            } else {
                                return Some(Token::Pipe);
                            }
                        }
                        b'=' if !expect_path => {}
                        _ => {
                            self.push(b);
                        }
                    }
                }
            }
        }
        if self.state.escape_next || self.state.quote.is_some() {
            None
        } else {
            Some(if let Some(ref acc) = self.acc {
                Token::Path(acc.clone())
            } else {
                Token::Eps
            })
        }
    }
}

pub struct Parser {
    lexer: Lexer,
}

impl Parser {
    pub fn new() -> Self {
        Self {
            lexer: Lexer::new(),
        }
    }

    pub fn feed(&mut self, buf: Vec<u8>) -> anyhow::Result<Vec<Box<dyn Cmd>>> {
        let mut cmds = vec![];
        self.lexer.feed(buf);
        while let Some(token) = self.lexer.next() {
            match token {
                Token::Eps => {
                    return Ok(cmds);
                }
                Token::Path(p) => {
                    let pos = p.iter().position(|&r| r == b'=');
                    let pos = if let Some(pos) = pos { pos } else { 0 };
                    if pos > 0 {
                        let lhs = (&p.as_slice()[..pos]).to_vec();
                        let rhs = (&p.as_slice()[pos + 1..]).to_vec();
                        cmds.push(Box::new(SetEnvCmd::new(lhs, rhs)));
                    } else {
                        let mut args: Vec<Vec<u8>> = vec![];
                        while let Some(out) = self.lexer.next() {
                            match out {
                                Token::Eps | Token::Pipe => {
                                    break;
                                }
                                Token::Path(p) => {
                                    args.push(p);
                                }
                            }
                        }
                        cmds.push(match p.as_slice() {
                            b"cat" => Box::new(CatCmd::new(args)),
                            b"echo" => Box::new(EchoCmd::new(args)),
                            b"exit" => Box::new(ExitCmd::new(args)),
                            b"wc" => Box::new(WcCmd::new(args)),
                            b"pwd" => Box::new(PwdCmd::new(args)),
                            p => Box::new(ProcCmd::new(p.to_vec(), args)),
                        })
                    }
                }
                Token::Pipe => {
                    return Err(anyhow::anyhow!("syntax error near unexpected token '|'"));
                }
            }
        }
        unreachable!()
    }
}

#[cfg(test)]
mod tests {
    use crate::syntax::Token;

    use super::Lexer;

    #[test]
    fn test_lexer_simple() {
        let line = b"    awda75 awdwe llllplk  ".to_vec();
        let mut lexer = Lexer::new();
        assert_eq!(lexer.next(), Some(Token::Eps));
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"awda75".to_vec())));
        assert_eq!(lexer.next(), Some(Token::Path(b"awdwe".to_vec())));
        assert_eq!(lexer.next(), Some(Token::Path(b"llllplk".to_vec())));
        assert_eq!(lexer.next(), Some(Token::Eps));
    }

    #[test]
    fn test_lexer_expect_next() {
        let mut line = b"    awda75 \"wdwdw  ".to_vec();
        let mut lexer = Lexer::new();
        assert_eq!(lexer.next(), Some(Token::Eps));
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"awda75".to_vec())));
        assert_eq!(lexer.next(), None);
        assert_eq!(lexer.next(), None);
        line = b" awdwd\"".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"wdwdw   awdwd".to_vec())));
        assert_eq!(lexer.next(), Some(Token::Eps));
        line = b" \'wdwdw  ".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), None);
        line = b" awdwd\'".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"wdwdw   awdwd".to_vec())));
        assert_eq!(lexer.next(), Some(Token::Eps));
    }
}
