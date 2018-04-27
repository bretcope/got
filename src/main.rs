pub mod parsing;
pub mod profile;

#[macro_use]
extern crate failure;

use std::process;

fn main() {
    let content = match profile::FileContent::load("sample_profile") {
        Ok(content) => content,
        Err(err) => panic!("{}", err),
    };

    let mut lexer = parsing::Lexer::new(&content);
    loop
    {
        match lexer.advance() {
            Ok(token) => {
                println!("{:?} {}", token.token_type, match token.value { Some(ref value) => format!("\"{}\"", value), None => String::new() });
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