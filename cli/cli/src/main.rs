use std::{
    collections::VecDeque,
    io::{self, Read, Write},
};

mod cmd;
mod syntax;

use anyhow::Ok;
use cmd::Cmd;
use syntax::Parser;

struct Prompt {
    buf: String,
}

impl Prompt {
    fn new() -> Self {
        Self { buf: String::new() }
    }

    fn next(&mut self) -> anyhow::Result<&[u8]> {
        self.buf.clear();
        print!("> ");
        io::stdout().flush()?;
        let n = io::stdin().read_line(&mut self.buf)?;
        if n == 0 {
            Err(anyhow::anyhow!("EOF"))
        } else {
            /* n includes endline symbol. */
            Ok(&self.buf.as_bytes()[..n - 1])
        }
    }
}

fn parse_cmds(prompt: &mut Prompt) -> anyhow::Result<Vec<Box<dyn Cmd>>> {
    let mut parser = Parser::new();
    loop {
        let (cmds, next_parser) = parser.feed(prompt.next()?.to_owned());
        if cmds.is_some() {
            return cmds.unwrap();
        }
        parser = next_parser.unwrap();
    }
}

fn process(cmds: Vec<Box<dyn Cmd>>) -> anyhow::Result<()> {
    let mut prev = VecDeque::<u8>::new();
    let mut next = VecDeque::<u8>::new();
    let n = cmds.len();
    for (i, mut cmd) in cmds.into_iter().enumerate() {
        let r: &mut dyn Read = if i == 0 { &mut io::stdin() } else { &mut prev };
        let w: &mut dyn Write = if i == n - 1 {
            &mut io::stdout()
        } else {
            &mut next
        };
        (*cmd).run(r, w)?;
        std::mem::swap(&mut prev, &mut next);
    }
    Ok(())
}

fn main() {
    let mut prompt = Prompt::new();
    loop {
        let cmds = parse_cmds(&mut prompt);
        if let Err(err) = cmds {
            println!("[x] parse error: {}", err);
            continue;
        }
        let cmds = cmds.unwrap();
        if let Err(err) = process(cmds) {
            println!("[x] error: {}", err);
        }
        io::stdout().flush().expect("flush works");
    }
}
