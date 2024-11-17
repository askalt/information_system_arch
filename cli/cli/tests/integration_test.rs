use rand::{
    distributions::{Alphanumeric, DistString},
    rngs::StdRng,
    Rng, SeedableRng,
};
use std::path::Path;
use std::{
    collections::VecDeque,
    env,
    fs::{create_dir, remove_dir_all, File},
    path::PathBuf,
};

use cli::cmd::{cd::CdCmd, env::Env, ls::LsCmd, Cmd};

fn get_rng(seed: u64) -> StdRng {
    return StdRng::seed_from_u64(seed);
}

fn get_random_string<R: Rng + ?Sized>(rng: &mut R, len: usize) -> String {
    return Alphanumeric.sample_string(rng, len);
}

fn get_default_env() -> Env {
    return Env::new(env::current_dir().unwrap());
}

fn check_cmd(mut cmd: Box<dyn Cmd>, env: &mut Env, expected_result: &[u8], is_ok: bool) {
    let mut input_buffer = VecDeque::<u8>::new();
    let mut output_buffer = Vec::<u8>::new();

    let res = (*cmd).run(&mut input_buffer, &mut output_buffer, env);
    assert_eq!(is_ok, res.is_ok());
    assert_eq!(expected_result, output_buffer);
}

#[test]
fn test_filesystem_navigation() {
    let mut rng = get_rng(239);
    let dir_name_len = 20;

    let dir_names = vec![
        get_random_string(&mut rng, dir_name_len),
        get_random_string(&mut rng, dir_name_len),
    ];

    for dir_name in dir_names.iter() {
        let _ = create_dir(&dir_name);
        let _ = File::create(Path::new(dir_name).join("a.txt"));
    }

    let mut env = get_default_env();
    let initial_dir = PathBuf::from(env.get_dir());

    // ls: single path
    check_cmd(
        Box::new(LsCmd::new(vec![dir_names[0].clone().into_bytes()])),
        &mut env,
        b"a.txt\n".as_slice(),
        true,
    );

    // ls: multiple paths
    check_cmd(
        Box::new(LsCmd::new(
            dir_names
                .iter()
                .map(|s| s.clone().into_bytes())
                .collect::<Vec<_>>(),
        )),
        &mut env,
        format!("{}:\na.txt\n\n{}:\na.txt\n", &dir_names[0], &dir_names[1]).as_bytes(),
        true,
    );

    let invalid_dir = get_random_string(&mut rng, dir_name_len);
    let invalid_cmd_args = vec![invalid_dir.clone().into_bytes()];

    // ls: incorrect
    check_cmd(
        Box::new(LsCmd::new(invalid_cmd_args.clone())),
        &mut env,
        format!(
            "ls: cannot access '{}': No such file or directory\n",
            &invalid_dir
        )
        .as_bytes(),
        true,
    );

    // cd: incorrect
    check_cmd(
        Box::new(CdCmd::new(invalid_cmd_args.clone())),
        &mut env,
        format!("cd: '{}': No such file or directory\n", &invalid_dir).as_bytes(),
        true,
    );

    assert_eq!(initial_dir, env.get_dir());

    // cd: absolute path
    check_cmd(
        Box::new(CdCmd::new(vec![dir_names[0].clone().into_bytes()])),
        &mut env,
        &[],
        true,
    );

    assert_eq!(initial_dir.join(&dir_names[0]), env.get_dir());

    // cd: relative path
    check_cmd(
        Box::new(CdCmd::new(vec![b"../".to_vec()])),
        &mut env,
        &[],
        true,
    );

    assert_eq!(initial_dir, env.get_dir());

    // cd: without arguments
    check_cmd(Box::new(CdCmd::new(vec![])), &mut env, &[], true);

    let home_dir = homedir::my_home().unwrap();
    if let Some(home_dir_path) = home_dir {
        assert_eq!(home_dir_path, env.get_dir());
    }

    for dir_name in dir_names.iter() {
        let _ = remove_dir_all(initial_dir.join(&dir_name));
    }
}
