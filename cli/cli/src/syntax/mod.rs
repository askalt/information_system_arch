#[derive(PartialEq, Debug)]
enum Token<'a> {
    Eps,
    Path(&'a [u8]),
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
    out: Vec<u8>,
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
            out: vec![],
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

    fn get_out(&mut self) -> Token {
        let acc = self.acc.as_mut().unwrap();
        self.out.clear();
        self.out.extend(acc.iter().cloned());
        self.acc = None;
        Token::Path(&self.out)
    }

    fn next(&mut self) -> Option<Token> {
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
                        if self.acc.is_some() {
                            return Some(self.get_out());
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
            Some(if self.acc.is_some() {
                self.get_out()
            } else {
                Token::Eps
            })
        }
    }
}

pub trait Command {}

struct Parser {
    lexer: Lexer,
}

impl Parser {
    fn new() -> Self {
        Self {
            lexer: Lexer::new(),
        }
    }

    fn feed(&mut self, buf: Vec<u8>) -> Option<Vec<Box<dyn Command>>> {
        self.lexer.feed(buf);
        let mut acc: Vec<Vec<u8>> = vec![];
        while let Some(out) = self.lexer.next() {
            match out {
                Token::Eps => {
                    if acc.is_empty() {
                        return Some(vec![]);
                    } else {
                        match acc[0].as_slice() {
                            b"cat" => {}
                            b"echo" => {}
                            b"wc" => {}
                            b"pwd" => {}
                            b"exit" => {}
                            p => {}
                        }
                    }
                }
                Token::Path(p) => {
                    acc.push(p.to_owned());
                }
            }
        }
        None
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
        assert_eq!(lexer.next(), Some(Token::Path(b"awda75")));
        assert_eq!(lexer.next(), Some(Token::Path(b"awdwe")));
        assert_eq!(lexer.next(), Some(Token::Path(b"llllplk")));
        assert_eq!(lexer.next(), Some(Token::Eps));
    }

    #[test]
    fn test_lexer_expect_next() {
        let mut line = b"    awda75 \"wdwdw  ".to_vec();
        let mut lexer = Lexer::new();
        assert_eq!(lexer.next(), Some(Token::Eps));
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"awda75")));
        assert_eq!(lexer.next(), None);
        assert_eq!(lexer.next(), None);
        line = b" awdwd\"".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"wdwdw   awdwd")));
        assert_eq!(lexer.next(), Some(Token::Eps));
        line = b" \'wdwdw  ".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), None);
        line = b" awdwd\'".to_vec();
        lexer.feed(line);
        assert_eq!(lexer.next(), Some(Token::Path(b"wdwdw   awdwd")));
        assert_eq!(lexer.next(), Some(Token::Eps));
    }
}
