pub mod parsing;
pub mod profile;

#[macro_use]
extern crate failure;

fn main() {
    let content = match profile::FileContent::load("sample_profile") {
        Ok(content) => content,
        Err(err) => panic!("{}", err),
    };

    let mut lexer = parsing::Lexer::new(&content);
    lexer.peek_type();
    let _ = lexer.advance();
}