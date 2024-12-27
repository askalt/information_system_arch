use std::collections::HashMap;

use super::EnvAssign;

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

    pub fn with_assigns(
        &self,
        assigns: Vec<EnvAssign>,
    ) -> impl IntoIterator<Item = (String, String)> {
        self.0
            .iter()
            .map(|(k, v)| {
                (
                    String::from_utf8(k.clone()).unwrap(),
                    String::from_utf8(v.clone()).unwrap(),
                )
            })
            .collect::<Vec<_>>()
            .into_iter()
            .chain(assigns.into_iter().map(|it| {
                (
                    String::from_utf8(it.lvalue).unwrap(),
                    String::from_utf8(it.rvalue).unwrap(),
                )
            }))
    }
}
