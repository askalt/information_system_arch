use std::{
    fs::File,
    io::{BufRead, BufReader},
    path::Path,
};

use clap::{arg, command, Parser};

use super::Cmd;

pub struct GrepCmd {
    args: Vec<Vec<u8>>,
}

impl GrepCmd {
    pub fn new(args: Vec<Vec<u8>>) -> Self {
        Self { args: args }
    }

    fn match_line(&self, line: &str, args: &Args) -> bool {
        let line = if args.case_insensitive {
            line.to_lowercase()
        } else {
            line.to_string()
        };

        let pattern = if args.case_insensitive {
            args.pattern.to_lowercase()
        } else {
            args.pattern.clone()
        };

        if args.whole_word {
            // Wrap the regex with word boundaries if `-w` is specified.
            let word_boundary_pattern = format!(r"\b{}\b", regex::escape(&args.pattern));
            let word_boundary_regex = regex::Regex::new(&word_boundary_pattern).unwrap();
            word_boundary_regex.is_match(&line)
        } else {
            let regex = regex::Regex::new(&pattern).unwrap();
            regex.is_match(&line)
        }
    }

    fn raw_run(
        &mut self,
        r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        args: &Args,
    ) -> anyhow::Result<()> {
        let reader = BufReader::new(r);
        /* Write all lines until limit. */
        let mut lim_to_write = -1;
        for (i, line) in reader.lines().enumerate() {
            let line = line?;
            let matched = self.match_line(&line, &args);
            if matched || i as i32 <= lim_to_write {
                writeln!(w, "{}", line)?;
                if matched {
                    lim_to_write = i as i32 + args.count as i32;
                }
            }
        }

        Ok(())
    }
}

#[derive(Parser, Debug)]
#[command(name = "grep", about = "A simple grep-like command-line tool")]
struct Args {
    /// Perform case-insensitive search.
    #[arg(short = 'i')]
    case_insensitive: bool,

    /// Match the whole word.
    #[arg(short = 'w')]
    whole_word: bool,

    /// Number of lines to print after matched.
    #[arg(short = 'A')]
    count: usize,

    /// The pattern to search for.
    pattern: String,

    /// The file to search in.
    file: String,
}

impl Cmd for GrepCmd {
    fn run(
        &mut self,
        _r: &mut dyn std::io::Read,
        w: &mut dyn std::io::Write,
        _env: &mut super::env::Env,
    ) -> anyhow::Result<()> {
        let args = Args::try_parse_from(
            std::iter::once("grep".into()).chain(
                self.args
                    .iter()
                    .map(|it| String::from_utf8(it.to_vec()).unwrap()),
            ),
        )?;
        let path = Path::new(&args.file);
        if !path.exists() {
            anyhow::bail!("File does not exist: {}", args.file);
        }

        let mut file = File::open(&args.file)?;
        self.raw_run(&mut file, w, &args)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn test_simple_match() {
        let mut input = Cursor::new("This is a test line with the pattern.\nAnother line.\n");
        let mut output = Vec::new();
        let mut grep_cmd = GrepCmd::new(vec![]);

        let result = grep_cmd.raw_run(
            &mut input,
            &mut output,
            &Args {
                case_insensitive: false,
                whole_word: false,
                pattern: "pattern".into(),
                file: "".into(),
                count: 0,
            },
        );

        assert!(result.is_ok());
        let output = String::from_utf8(output).unwrap();
        assert_eq!(output, "This is a test line with the pattern.\n");
    }

    #[test]
    fn test_case_insensitive() {
        let mut input = Cursor::new("This is a test line with the pattern.\nAnother line.\n");
        let mut output = Vec::new();
        let mut grep_cmd = GrepCmd::new(vec![]);

        let result = grep_cmd.raw_run(
            &mut input,
            &mut output,
            &Args {
                case_insensitive: true,
                whole_word: false,
                pattern: "PATTERN".into(),
                file: "".into(),
                count: 0,
            },
        );

        assert!(result.is_ok());
        let output = String::from_utf8(output).unwrap();
        assert_eq!(output, "This is a test line with the pattern.\n");
    }

    #[test]
    fn test_whole_word() {
        let mut input = Cursor::new("This is a test line with pattern matching.\nAnother line.\n");
        let mut output = Vec::new();
        let mut grep_cmd = GrepCmd::new(vec![]);

        let result = grep_cmd.raw_run(
            &mut input,
            &mut output,
            &mut &Args {
                case_insensitive: false,
                whole_word: true,
                pattern: "pattern".into(),
                file: "".into(),
                count: 0,
            },
        );

        assert!(result.is_ok());
        let output = String::from_utf8(output).unwrap();
        assert_eq!(output, "This is a test line with pattern matching.\n");
    }

    #[test]
    fn test_after_count() {
        let mut input = Cursor::new(
            "Line 1\nThis line has the pattern.\nLine 3\nLine 4\nAnother pattern line.\nLine 6\n",
        );
        let mut output = Vec::new();
        let mut grep_cmd = GrepCmd::new(vec![]);

        let result = grep_cmd.raw_run(
            &mut input,
            &mut output,
            &mut Args {
                case_insensitive: false,
                whole_word: true,
                pattern: "pattern".into(),
                file: "".into(),
                count: 1,
            },
        );

        assert!(result.is_ok());
        let output = String::from_utf8(output).unwrap();
        assert_eq!(
            output,
            "This line has the pattern.\nLine 3\nAnother pattern line.\nLine 6\n"
        );
    }

    #[test]
    fn test_no_match() {
        let mut input = Cursor::new("This is a test line with the pattern.\nAnother line.\n");
        let mut output = Vec::new();
        let mut grep_cmd = GrepCmd::new(vec![]);

        let result = grep_cmd.raw_run(
            &mut input,
            &mut output,
            &mut Args {
                case_insensitive: false,
                whole_word: true,
                pattern: "notfound".into(),
                file: "".into(),
                count: 1,
            },
        );

        assert!(result.is_ok());
        let output = String::from_utf8(output).unwrap();
        assert!(output.is_empty());
    }
}
