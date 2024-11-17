use super::{env::Env, Cmd};
use core::str;
use std::{fs, os::unix::ffi::OsStrExt, path::Path};

pub struct LsCmd {
    args: Vec<Vec<u8>>,
}

impl LsCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args }
    }
}

impl Cmd for LsCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        env: &mut Env,
    ) -> anyhow::Result<()> {
        if self.args.is_empty() {
            self.args.push(vec![]);
        }

        let need_header = self.args.len() > 1;

        for (i, arg) in self.args.iter().enumerate() {
            if i > 0 {
                w.write(b"\n")?;
            }

            let path_str = str::from_utf8(arg.as_slice())?;
            let combined_path = env.get_dir().join(Path::new(path_str));
            if !combined_path.exists() {
                w.write(
                    format!(
                        "ls: cannot access '{}': No such file or directory\n",
                        &path_str
                    )
                    .as_bytes(),
                )?;
            } else {
                if need_header {
                    w.write(&path_str.as_bytes())?;
                    w.write(b":\n")?;
                }

                if combined_path.is_dir() {
                    for (j, entry) in fs::read_dir(combined_path)?.enumerate() {
                        if j > 0 {
                            w.write(b" ")?;
                        }
                        w.write(entry?.path().file_name().unwrap().as_bytes())?;
                    }
                } else {
                    w.write(combined_path.file_name().unwrap().as_bytes())?;
                }

                w.write(b"\n")?;
            }
        }

        Ok(())
    }
}
