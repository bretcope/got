mod error;
pub use self::error::ParserError;

mod tokens;
pub use self::tokens::{Token, TokenType};

mod lexer;

mod nodes;
pub use self::nodes::*;

pub enum SyntaxElement<'a> {
    Token(&'a Token<'a>),
    Node(&'a Node<'a>),
}
