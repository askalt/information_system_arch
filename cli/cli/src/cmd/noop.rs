use super::Cmd;
pub struct NoopCmd {}

impl NoopCmd {
    pub fn new() -> Self {
        Self {}
    }
}

impl Cmd for NoopCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        _w: &mut dyn std::io::Write,
    ) -> anyhow::Result<()> {
        Ok(())
    }
}
