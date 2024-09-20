use std::{env, os::unix::ffi::OsStrExt};

use super::Cmd;

pub struct PwdCmd {}

impl PwdCmd {
    pub fn new(_args: Vec<Vec<u8>>) -> Self {
        Self {}
    }
}

impl Cmd for PwdCmd {
    fn run(&mut self, w: &mut dyn std::io::Write) -> anyhow::Result<()> {
        let dir = env::current_dir()?;
        w.write(dir.as_os_str().as_bytes())?;
        Ok(())
    }
}
