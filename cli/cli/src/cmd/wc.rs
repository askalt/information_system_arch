use core::str;
use std::fs::File;
use std::io::{self, Read};
use std::ops::AddAssign;
use std::path::Path;

use super::Cmd;

pub struct WcCmd {
    args: Vec<Vec<u8>>,
}

impl WcCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args }
    }
}

#[derive(Default)]
struct Stat {
    lines: usize,
    words: usize,
    bytes: usize,
}

impl AddAssign<&Self> for Stat {
    fn add_assign(&mut self, rhs: &Self) {
        self.lines += rhs.lines;
        self.words += rhs.words;
        self.bytes += rhs.bytes;
    }
}

impl WcCmd {
    const STDIN_FNAME: &[u8] = b"-";

    fn get_stat(file: &mut dyn Read) -> anyhow::Result<Stat> {
        let mut stat = Stat::default();
        let mut buf = vec![0_u8; 1024];
        let mut is_space = true;
        loop {
            let n = file.read(&mut buf)?;
            if n == 0 {
                break;
            }
            for i in 0..n {
                is_space = if match buf[i] {
                    b'\n' => {
                        stat.lines += 1;
                        true
                    }
                    b' ' => true,
                    _ => false,
                } {
                    if !is_space {
                        stat.words += 1;
                    }
                    true
                } else {
                    false
                }
            }
            stat.bytes += n;
        }
        if !is_space {
            stat.words += 1;
        }
        Ok(stat)
    }

    fn write_stat(w: &mut dyn std::io::Write, stat: &Stat, width: usize) -> anyhow::Result<()> {
        w.write_fmt(format_args!("{:width$}", stat.lines))?;
        w.write_fmt(format_args!(" {:width$}", stat.words))?;
        w.write_fmt(format_args!(" {:width$}", stat.bytes))?;
        Ok(())
    }

    fn compute_number_width(file_names: &[Vec<u8>]) -> anyhow::Result<usize> {
        let mut width = 1_usize;
        let mut minimum_width = 1_usize;
        let mut regular_total = 0_usize;
        for file_name in file_names {
            if file_name == Self::STDIN_FNAME {
                minimum_width = 7;
            } else {
                let file = File::open(Path::new(str::from_utf8(file_name.as_slice())?))?;
                regular_total += file.metadata()?.len() as usize;
            }
        }
        while 10 <= regular_total {
            regular_total /= 10;
            width += 1;
        }
        Ok(width.max(minimum_width))
    }
}

impl Cmd for WcCmd {
    fn run(&mut self, r: &mut dyn std::io::Read, w: &mut dyn std::io::Write) -> anyhow::Result<()> {
        if self.args.is_empty() {
            self.args.push(Self::STDIN_FNAME.to_vec());
        }
        let width = Self::compute_number_width(&self.args)?;
        let mut total_stat = Stat::default();
        for path in self.args.iter() {
            let path_str = str::from_utf8(path.as_slice())?;
            let stat = if path == Self::STDIN_FNAME {
                Self::get_stat(r)?
            } else {
                let mut file = File::open(Path::new(path_str))?;
                Self::get_stat(&mut file)?
            };
            Self::write_stat(w, &stat, width)?;
            w.write(b" ")?;
            w.write(path.as_slice())?;
            w.write(b"\n")?;
            w.flush()?;
            total_stat += &stat;
        }
        if self.args.len() > 1 {
            Self::write_stat(w, &total_stat, width)?;
            w.write(b" total\n")?;
            w.flush()?;
        }
        Ok(())
    }
}
