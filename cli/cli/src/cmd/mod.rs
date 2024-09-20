use std::io::Write;

pub mod echo;
pub mod exit;
pub mod proc;
pub mod pwd;
pub mod wc;

pub trait Command {
    fn run(&mut self, w: &mut dyn Write) -> anyhow::Result<()>;
}
