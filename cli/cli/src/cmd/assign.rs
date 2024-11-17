use super::{env::Env, Cmd, EnvAssign};
pub struct AssignCmd {
    assigns: Vec<EnvAssign>,
}

impl AssignCmd {
    pub fn new(assigns: Vec<EnvAssign>) -> Self {
        Self { assigns: assigns }
    }
}

impl Cmd for AssignCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        _w: &mut dyn std::io::Write,
        env: &mut Env,
    ) -> anyhow::Result<()> {
        for assign in self.assigns.iter() {
            env.set_var(assign.lvalue.clone(), assign.rvalue.clone());
        }
        Ok(())
    }
}
