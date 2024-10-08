use std::io::{Read, Write};

pub mod cat;
pub mod echo;
pub mod exit;
pub mod proc;
pub mod pwd;
pub mod setenv;
pub mod wc;

pub trait Cmd {
    fn run(&mut self, r: &mut dyn Read, w: &mut dyn Write) -> anyhow::Result<()>;
}
