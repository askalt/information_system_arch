use super::{env::Env, Cmd};
use core::str;
use homedir::my_home;
use std::path::Path;

pub struct CdCmd {
    args: Vec<Vec<u8>>,
}

impl CdCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args }
    }
}

impl Cmd for CdCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        env: &mut Env,
    ) -> anyhow::Result<()> {
        if self.args.is_empty() {
            // Go to the home directory
            env.set_dir(my_home()?.unwrap());
        } else {
            let path_str = str::from_utf8(self.args[0].as_slice())?;
            let combined_path = env.get_dir().join(Path::new(path_str));

            if !combined_path.exists() {
                w.write(format!("cd: '{}': No such file or directory\n", &path_str).as_bytes())?;
            } else {
                env.set_dir(combined_path.canonicalize().unwrap());
            }
        }

        Ok(())
    }
}
