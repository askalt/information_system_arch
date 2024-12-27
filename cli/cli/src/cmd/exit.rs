use std::process;

use super::{env::Env, Cmd};

pub struct ExitCmd {}

impl ExitCmd {
    pub fn new(_args: Vec<Vec<u8>>) -> Self {
        Self {}
    }
}

impl Cmd for ExitCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        _env: &mut Env,
    ) -> anyhow::Result<()> {
        w.write(b"\n")?;
        process::exit(0);
    }
}
