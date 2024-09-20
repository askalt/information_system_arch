use crate::cmd::{cat::CatCmd, echo::EchoCmd, exit::ExitCmd, pwd::PwdCmd, Cmd};

#[derive(PartialEq, Debug)]
enum Token {
    Eps,
    Path(Vec<u8>),
}

struct LexerState {
    escape_next: bool,
    quote: Option<u8>,
}

struct Lexer {
    state: LexerState,
    pos: usize,
    buf: Vec<u8>,
    acc: Option<Vec<u8>>,
}

impl Lexer {
    fn new() -> Self {
        Self {
            state: LexerState {
                escape_next: false,
                quote: None,
            },
            pos: 0,
            buf: vec![],
            acc: None,
        }
    }

    fn feed(&mut self, buf: Vec<u8>) {
        self.pos = 0;
        self.buf = buf;
    }

    fn push(&mut self, b: u8) {
        match self.acc {
            Some(ref mut acc) => {
                acc.push(b);
            }
            None => {
                self.acc = Some(vec![b]);
            }
        }
    }

    fn next(&mut self) -> Option<Token> {
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
                    if b == b' ' {
                        if let Some(ref acc) = self.acc {
                            return Some(Token::Path(acc.clone()));
                        }
                    } else if b == b'\'' || b == b'\"' {
                        self.state.quote = Some(b);
                    } else {
                        self.push(b);
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

    pub fn feed(&mut self, buf: Vec<u8>) -> Option<Vec<Box<dyn Cmd>>> {
        self.lexer.feed(buf);
        let cmd_name = self.lexer.next()?;
        match cmd_name {
            Token::Eps => Some(vec![]),
            Token::Path(p) => {
                let mut args: Vec<Vec<u8>> = vec![];
                while let Some(out) = self.lexer.next() {
                    match out {
                        Token::Eps => {
                            break;
                        }
                        Token::Path(p) => {
                            args.push(p);
                        }
                    }
                }
                Some(vec![match p.as_slice() {
                    b"cat" => Box::new(CatCmd::new(args)),
                    b"echo" => Box::new(EchoCmd::new(args)),
                    b"exit" => Box::new(ExitCmd::new(args)),
                    b"wc" => todo!(),
                    b"pwd" => Box::new(PwdCmd::new(args)),
                    _p => todo!(),
                }])
            }
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
