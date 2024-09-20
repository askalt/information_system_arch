use super::Cmd;
use core::str;
use std::fs::File;
use std::io::Read;
use std::path::Path;

pub struct CatCmd {
    args: Vec<Vec<u8>>,
}

impl CatCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args }
    }
}

impl Cmd for CatCmd {
    fn run(&mut self, w: &mut dyn std::io::Write) -> anyhow::Result<()> {
        let mut buf = vec![0_u8; 1024];
        for arg in self.args.iter() {
            let path_str = str::from_utf8(arg.as_slice())?;
            let mut file = File::open(Path::new(path_str))?;
            loop {
                let n = file.read(&mut buf)?;
                if n == 0 {
                    break;
                }
                w.write(&buf[..n])?;
            }
        }
        Ok(())
    }
}
