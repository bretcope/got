use profile::FileContent;

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

#[derive(Debug)]
pub struct Token<'a> {
    pub token_type: TokenType,
    pub content: &'a FileContent,
    pub trivia_start: usize,
    pub text_start: usize,
    pub text_end: usize,
    pub value: Option<String>,
}
