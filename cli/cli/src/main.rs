use std::io::{self, Write};

mod cmd;
mod syntax;

use syntax::Parser;

struct Prompt {
    buf: String,
}

impl Prompt {
    fn new() -> Self {
        Self { buf: String::new() }
    }

    fn next(&mut self) -> anyhow::Result<&[u8]> {
        print!("> ");
        io::stdout().flush()?;
        let n = io::stdin().read_line(&mut self.buf)?;
        /* n includes endline symbol. */
        Ok(&self.buf.as_bytes()[..n - 1])
    }
}

fn main() {
    let mut prompt = Prompt::new();
    loop {
        let mut parser = Parser::new();
        let mut cmds = vec![];
        loop {
            let nxt_line = prompt.next().expect("read is ok").to_owned();
            if let Some(parsed_cmds) = parser.feed(nxt_line) {
                cmds = parsed_cmds;
                break;
            }
        }
        if cmds.len() != 1 {
            println!("[x] multi commands are not supported");
            continue;
        }
        let res = cmds[0].run(&mut io::stdout());
        if let Err(err) = res {
            println!("[x] error: {}", err);
        }
    }
}
