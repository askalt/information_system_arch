use std::collections::HashMap;

pub struct Env(HashMap<Vec<u8>, Vec<u8>>);

impl Env {
    pub fn new() -> Self {
        Self(Default::default())
    }

    pub fn set(&mut self, k: Vec<u8>, v: Vec<u8>) {
        self.0.insert(k, v);
    }

    pub fn get(&self, k: Vec<u8>) -> Vec<u8> {
        self.0.get(&k).cloned().unwrap_or_default()
    }
}
