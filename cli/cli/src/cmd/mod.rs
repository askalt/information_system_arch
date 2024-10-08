use std::io::{Read, Write};

use env::Env;

pub mod cat;
pub mod echo;
pub mod exit;
pub mod proc;
pub mod pwd;
pub mod wc;
pub mod assign;
pub mod env;

#[derive(Clone)]
pub struct EnvAssign {
    lvalue: Vec<u8>,
    rvalue: Vec<u8>,
}

impl EnvAssign {
    pub fn new(lvalue: Vec<u8>, rvalue: Vec<u8>) -> Self {
        Self { lvalue: lvalue, rvalue: rvalue }
    }
}

pub trait Cmd {
    fn run(&mut self, r: &mut dyn Read, w: &mut dyn Write, env: &mut Env) -> anyhow::Result<()>;
}
