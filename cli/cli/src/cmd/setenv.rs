use super::Cmd;

pub struct SetEnvCmd {
    lhs: Vec<u8>,
    rhs: Vec<u8>,
}

impl SetEnvCmd {
    pub fn new(lhs: Vec<u8>, rhs: Vec<u8>) -> Self {
        Self { lhs, rhs }
    }
}

impl Cmd for SetEnvCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        _w: &mut dyn std::io::Write,
    ) -> anyhow::Result<()> {
        Ok(())
    }
}
