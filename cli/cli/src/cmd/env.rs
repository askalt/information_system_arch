use std::collections::HashMap;
use std::path::{Path, PathBuf};

pub struct Env {
    vars: HashMap<Vec<u8>, Vec<u8>>,
    current_dir: PathBuf,
}

impl Env {
    pub fn new(initial_dir: PathBuf) -> Self {
        Self {
            vars: Default::default(),
            current_dir: initial_dir,
        }
    }

    pub fn set_var(&mut self, k: Vec<u8>, v: Vec<u8>) {
        self.vars.insert(k, v);
    }

    pub fn get_var(&self, k: Vec<u8>) -> Vec<u8> {
        self.vars.get(&k).cloned().unwrap_or_default()
    }

    pub fn get_dir(&self) -> &Path {
        return self.current_dir.as_path();
    }

    pub fn set_dir(&mut self, new_dir: PathBuf) {
        self.current_dir = new_dir;
    }
}
