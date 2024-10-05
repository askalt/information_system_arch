use super::Cmd;

pub struct EchoCmd {
    args: Vec<Vec<u8>>,
}

impl EchoCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args }
    }
}

impl Cmd for EchoCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
    ) -> anyhow::Result<()> {
        for (i, arg) in self.args.iter().enumerate() {
            if i > 0 {
                w.write(b" ")?;
            }
            w.write(&arg)?;
        }
        w.write(b"\n")?;
        Ok(())
    }
}
