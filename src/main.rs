pub mod colors;
pub mod parsing;
pub mod profile;

#[macro_use]
extern crate failure;

#[cfg(windows)]
extern crate winapi;

use std::process;

fn main() {
    enable_console_colors();

    let content = match profile::FileContent::load("sample_profile") {
        Ok(content) => content,
        Err(err) => panic!("{}", err),
    };

//    let mut lexer = parsing::Lexer::new(&content);
//    loop
//    {
//        match lexer.advance() {
//            Ok(token) => {
//                println!("{:?} {}", token.token_type, match token.value { Some(ref value) => format!("\"{}\"", value), None => String::new() });
//                if token.token_type == parsing::TokenType::EndOfInput {
//                    break;
//                }
//            },
//            Err(err) => {
//                println!("{}", err);
//                process::exit(1);
//            }
//        };
//    }

    let file_node = match parsing::parse_configuration_file(&content) {
        Ok(node) => node,
        Err(err) => {
            println!("{}", err);
            process::exit(1);
        },
    };

    println!("{:?}", file_node);
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
