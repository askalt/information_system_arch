use std::os::unix::ffi::OsStrExt;

use super::{env::Env, Cmd};

pub struct PwdCmd {}

impl PwdCmd {
    pub fn new(_args: Vec<Vec<u8>>) -> Self {
        Self {}
    }
}

impl Cmd for PwdCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        env: &mut Env,
    ) -> anyhow::Result<()> {
        let dir = env.get_dir();
        w.write(dir.as_os_str().as_bytes())?;
        w.write(b"\n")?;
        Ok(())
    }
}
