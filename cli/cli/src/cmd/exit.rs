use std::process;

use super::Cmd;

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
    ) -> anyhow::Result<()> {
        w.write(b"\n")?;
        process::exit(0);
    }
}
