use core::str;
use std::io::Read;
use std::process::{Command, Stdio};

use super::env::Env;
use super::{Cmd, EnvAssign};

pub struct ProcCmd {
    cmd_path: Vec<u8>,
    args: Vec<Vec<u8>>,
    /* Assigns to perform before execution. */
    _assigns: Vec<EnvAssign>,
}

impl ProcCmd {
    pub fn new(cmd_path: Vec<u8>, args: Vec<Vec<u8>>, assigns: Vec<EnvAssign>) -> Self {
        Self {
            cmd_path,
            args,
            _assigns: assigns,
        }
    }
}

impl Cmd for ProcCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        _env: &mut Env,
    ) -> anyhow::Result<()> {
        let str_args = self
            .args
            .iter()
            .map(|it| str::from_utf8(it.as_slice()).map_err(anyhow::Error::from))
            .collect::<anyhow::Result<Vec<_>>>()?;
        let mut cmd = Command::new(str::from_utf8(self.cmd_path.as_slice())?)
            .args(str_args)
            .stdin(Stdio::inherit())
            .stdout(Stdio::piped())
            .spawn()?;
        let out = cmd.stdout.as_mut().unwrap();
        let mut buf = vec![0_u8; 1024];
        loop {
            let n = out.read(&mut buf)?;
            if n == 0 {
                break;
            }
            w.write(&buf[..n])?;
            w.flush()?;
        }
        Ok(())
    }
}
