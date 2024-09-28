use std::io::{self, Write};

mod cmd;
mod syntax;

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
        let nxt_line = prompt.next()?.to_owned();
        if let Some(parsed_cmds) = parser.feed(nxt_line) {
            return Ok(parsed_cmds);
        }
    }
}

fn main() {
    let mut prompt = Prompt::new();
    loop {
        let cmds = parse_cmds(&mut prompt);
        if let Err(err) = cmds {
            println!("[x] parse error: {}", err);
            continue;
        }
        let mut cmds = cmds.unwrap();
        if cmds.len() != 1 {
            println!("[x] multi commands are not supported");
            continue;
        }
        let res = cmds[0].run(&mut io::stdout());
        println!();
        if let Err(err) = res {
            println!("[x] error: {}", err);
        }
        io::stdout().flush().expect("flush works");
    }
}
