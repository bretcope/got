mod content;
mod error;
mod lexer;
mod tokens;

type Result<T> = std::result::Result<T, error::ParsingError>;
