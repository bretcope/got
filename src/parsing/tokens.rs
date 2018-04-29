use std::fmt;
use profile::FileContent;
use super::*;
use colors::*;

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub enum TokenType {
    Error,

    // whitespace/control
    StartOfInput,
    EndOfInput,
    EndOfLine,
    Indent,
    Outdent,

    // text
    Word,
    LineText,
    QuotedText,
    BlockText,

    // symbols
    Colon,
    GreaterThan,
}

pub struct Token<'a> {
    pub token_type: TokenType,
    pub content: &'a FileContent,
    pub trivia_start: usize,
    pub text_start: usize,
    pub text_end: usize,
    pub value: Option<String>,
}

impl<'a> Token<'a> {
    pub fn trivia(&self) -> &'a str {
        &self.content.text()[self.trivia_start..self.text_start]
    }

    pub fn text(&self) -> &'a str {
        &self.content.text()[self.text_start..self.text_end]
    }

    pub fn value_or_panic(&self, message: &str) -> &str {
        match &self.value {
            &Some(ref value) => value,
            &None => panic!(String::from(message)),
        }
    }

    pub fn value_or_text(&self) -> &str {
        match self.value {
            Some(ref value) => &value,
            None => self.text(),
        }
    }

    pub fn as_syntax_element(&'a self) -> SyntaxElement<'a> {
        SyntaxElement::Token(&self)
    }
}

impl<'a> fmt::Display for Token<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let pos = self.content.position_details(self.text_start);
        write!(f, "{}{:?} {}{}:{}{}", CYAN, self.token_type, GREY, pos.line_number, pos.column, RESET)?;

        if let &Some(ref value) = &self.value {
            write!(f, " \"{}\"", value)?;
        }

        writeln!(f)
    }
}

impl<'a> fmt::Debug for Token<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Display::fmt(&self, f)
    }
}
