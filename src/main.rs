pub mod colors;
pub mod parsing;
pub mod profile;

mod configuration;

mod text;
pub use text::*;

#[macro_use]
extern crate failure;

#[cfg(windows)]
extern crate winapi;

extern crate yaml_rust;

use std::process;
use configuration::MotConfig;

enum RunMode {
    Interactive,
    Simple,
}

fn main() {
    enable_console_colors();

    let args: Vec<String> = std::env::args().collect();

    if args.len() < 3 {
        eprintln!("Unexpected number of arguments. Be sure to use 'mot' instead of 'motcli'");
        std::process::exit(1);
    }

    let mode = match &args[1][..] {
        "-i" => RunMode::Interactive,
        "-s" => RunMode::Simple,
        _ => {
            eprintln!("Unsupported CLI mode. Be sure to use 'mot' instead of 'motcli'");
            std::process::exit(1)
        }
    };

    let exit_code = match &args[2].chars().next() {
        Some('-') => run_command(mode, &args[2..]),
        _ => run_directory_change(mode, &args[2..]),
    };

    std::process::exit(exit_code)
}

fn run_command(mode: RunMode, args: &[String]) -> i32 {
    println!("Running as interactive");
    1
}

fn run_directory_change(mode: RunMode, args: &[String]) -> i32 {
    if args.len() != 1 {
        eprintln!("mot expected just one argument");
        return 1
    }

    let conf_result = configuration::load_configuration();
    let conf = match conf_result {
        Ok(conf) => conf,
        Err(e) => {
            eprintln!("{}", e);
            return 1
        },
    };

    let dir = match conf.directories.get(&args[0]) {
        Some(dir) => dir, //println!("::EXEC::cd {}", dir.path),
        None => {
            eprintln!("Directory is not configured: {}", &args[0]);
            return 1
        }
    };

    print!("{}", dir.path);
    2
}

fn print_conf(conf: &MotConfig) {
    for dir in conf.directories.values() {
        println!("{}: {}", dir.name, dir.path)
    }
}

fn _debug_lexer(filename: &str) {
    let content = match profile::FileContent::load(filename) {
        Ok(content) => content,
        Err(err) => panic!("{}", err),
    };

    let mut lexer = parsing::Lexer::new(&content);
    loop
    {
        match lexer.advance() {
            Ok(token) => {
                print!("{}", token);
                if token.token_type == parsing::TokenType::EndOfInput {
                    break;
                }
            },
            Err(err) => {
                println!("{}", err);
                process::exit(1);
            }
        };
    }
}

fn _debug_parser(filename: &str) {
    let content = match profile::FileContent::load(filename) {
        Ok(content) => content,
        Err(err) => panic!("{}", err),
    };

    let file_node = match parsing::parse_configuration_file(&content) {
        Ok(node) => node,
        Err(err) => {
            println!("{}", err);
            process::exit(1);
        },
    };

    print!("{:?}", file_node);
}

fn enable_console_colors() {
    #[cfg(windows)]
    unsafe
    {
        let handle = winapi::um::processenv::GetStdHandle(winapi::um::winbase::STD_OUTPUT_HANDLE);
        if handle == winapi::um::handleapi::INVALID_HANDLE_VALUE {
            return;
        }

        let mut mode: u32 = 0;
        let result = winapi::um::consoleapi::GetConsoleMode(handle, &mut mode as *mut u32);
        if result == 0 {
            return;
        }

        mode |= winapi::um::wincon::ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        winapi::um::consoleapi::SetConsoleMode(handle, mode);
    }
}
