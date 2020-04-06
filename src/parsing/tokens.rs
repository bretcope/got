use super::content::FileContent;
use std::fmt;
use colors::*;

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub enum TokenType {
    Error,
    StartOfInput,
    EndOfInput,
    Identifier,
    Value,
    OpenBracket,
    AtOpenBracket,
    CloseBracket,
    Colon,
}

pub struct Token<'a> {
    pub token_type: TokenType,
    pub file_content: &'a FileContent,
    pub leading_trivia_start: usize,
    pub trailing_trivia_end: usize,
    pub text_start: usize,
    pub text_end: usize,
    value_: Option<String>,
}

impl<'a> Token<'a> {
    pub fn new(
        token_type: TokenType,
        file_content: &FileContent,
        leading_trivia_start: usize,
        trailing_trivia_end: usize,
        text_start: usize,
        text_end: usize,
        value: Option<String>) -> Token {

        match token_type {
            TokenType::Identifier |
            TokenType::Value => {
                if let None = value {
                    panic!("MOT Bug: token type `{:?}` created without a value.", token_type);
                }
            },
            _ => {
                if let Some(_) = value {
                    panic!("MOT Bug: token type `{:?}` created with a value.", token_type);
                }
            }
        };

        Token {
            token_type,
            file_content,
            leading_trivia_start,
            trailing_trivia_end,
            text_start,
            text_end,
            value_: value,
        }
    }

    pub fn leading_trivia(&self) -> &'a str {
        &self.file_content.text()[self.leading_trivia_start..self.text_start]
    }

    pub fn trailing_trivia(&self) -> &'a str {
        &self.file_content.text()[self.text_end..self.trailing_trivia_end]
    }

    pub fn text(&self) -> &'a str {
        &self.file_content.text()[self.text_start..self.text_end]
    }

    pub fn value(&self) -> Option<&str> {
        match self.value_ {
            Some(ref v) => Some(&v),
            None => None,
        }
    }

    pub fn value_or_text(&self) -> &str {
        match self.value() {
            Some(v) => v,
            None => self.text(),
        }
    }
}

impl<'a> fmt::Display for Token<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let pos = self.file_content.position_details(self.text_start);
        write!(f, "{}{:?} {}{}:{}{}", CYAN, self.token_type, GREY, pos.line_number, pos.column, RESET)?;

        if let Some(value) = self.value() {
            write!(f, " \"{}\"", value)?;
        }

        writeln!(f)
    }
}
