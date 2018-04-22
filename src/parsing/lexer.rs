use std::str::CharIndices;
use std::iter::Peekable;
use profile;
use super::*;

pub type LexerResult<'a> = Result<Box<Token<'a>>, ParserError>;

pub struct Lexer<'a> {
    content_: &'a profile::FileContent,
    content_iterator_: Peekable<CharIndices<'a>>,
    position_: usize,
    trivia_start_: usize,
    indent_level_: usize,
    line_start_: usize,
    line_spaces_: usize,
    last_token_type_: TokenType,
    next_result_: Option<LexerResult<'a>>,
}

impl<'a> Lexer<'a> {

    pub const SPACES_PER_INDENT: usize = 4;

    pub fn new(content: &'a profile::FileContent) -> Lexer<'a> {
        Lexer {
            content_: content,
            content_iterator_: content.text().char_indices().peekable(),
            position_: 0,
            trivia_start_: 0,
            indent_level_: 0,
            line_start_: 0,
            line_spaces_: 0,
            last_token_type_: TokenType::StartOfInput,
            next_result_: None,
        }
    }

    pub fn peek_type(&'a mut self) -> TokenType {
        if let None = self.next_result_ {
            self.next_result_ = Some(self.lex());
        };

        if let Some(ref result) = self.next_result_ {
            return match *result {
                Ok(ref token) => token.token_type,
                Err(_) => TokenType::Error,
            }
        }

        panic!("MOT Bug: BufferedResult was still 'None' after calling lex().");
    }

    pub fn advance(&mut self) -> LexerResult {
        let next = self.next_result_.take();
        match next {
            Some(result) => result,
            None => self.lex(),
        }
    }

    fn lex(&mut self) -> LexerResult {
        let last_type = self.last_token_type_;

        if let TokenType::EndOfInput = last_type {
            panic!("MOT Bug: Parser scanned past end of input.");
        }

        debug_assert_eq!(self.position_, self.iterator_position(), "Iterator does not match the lexer's current position at the beginning of lex().");

        self.consume_trivia();

        debug_assert_eq!(self.position_, self.iterator_position(), "Iterator does not match the lexer's current position after consuming trivia.");

        // either grab the next character, or run end-of-input lexing
        let ch = match self.content_iterator_.peek() {
            Some(&(_, ch)) => ch,
            None => return self.lex_end_of_input(),
        };

        // these are the tokens after which we need to be concerned about an indent or outdent
        match last_type {
            TokenType::StartOfInput |
            TokenType::EndOfLine |
            TokenType::Indent |
            TokenType::Outdent => {
                if self.position_ != self.line_start_ + self.line_spaces_ {
                    // the whitespace consumed at the start of the line doesn't match the number of spaces at the start of the line, which means there might be
                    // a tab character.
                    for (i, wc) in self.content_.text()[self.line_start_..self.position_].char_indices() {
                        if wc == '\t' {
                            return Err(ParserError::new(self.content_, self.line_start_ + i, "Tab character used for indentation. Four spaces must be used for indentation."))
                        }
                    }
                }

                if self.indent_level_ * Lexer::SPACES_PER_INDENT != self.line_spaces_ {
                    return self.lex_indentation();
                }
            },
            _ => {},
        };

        match ch {
            ':' => {
                self.content_iterator_.next();
                self.new_token(TokenType::Colon, None)
            },
            '>' => {
                self.content_iterator_.next();
                self.new_token(TokenType::GreaterThan, None)
            },
            '"' => self.lex_quoted_text(),
            '\r' |
            '\n' => match last_type {
                TokenType::GreaterThan => self.lex_block_text(),
                _ => self.lex_end_of_line(ch),
            },
            _ => {
                if let TokenType::Colon = last_type {
                    self.lex_line_text()
                } else if self.is_current_char_alpha() {
                    self.lex_word()
                } else {
                    self.err("Unexpected character.")
                }
            }
        }
    }

    fn consume_trivia(&mut self) {
        self.trivia_start_ = self.position_;
        let mut is_comment = false;

        while let Some(&(i, ch)) = self.content_iterator_.peek() {
            match ch {
                ' ' |
                '\t' => {
                    self.content_iterator_.next();
                },
                '\r' |
                '\n' => {
                    // The lexer isn't supposed to emit an EndOfLine token immediately after a StartOfInput, EndOfLine, or Outdent token.
                    match self.last_token_type_ {
                        TokenType::EndOfInput |
                        TokenType::StartOfInput |
                        TokenType::Outdent => {
                            self.content_iterator_.next();
                            if ch == '\r' {
                                if let Some(&(_, '\n')) = self.content_iterator_.peek() {
                                    self.content_iterator_.next();
                                }
                            }

                            self.start_new_line();
                            is_comment = false;
                        },
                        _ => break, // last token wasn't a StartOfInput, EndOfLine, or Outdent, so this new line is significant (don't consume it as trivia).
                    }
                },
                '#' => {
                    self.content_iterator_.next();
                    is_comment = true;
                },
                _ => {
                    if is_comment {
                        self.content_iterator_.next();
                    } else {
                        break;
                    }
                }
            };
        }

        self.position_ = self.iterator_position();
    }

    fn lex_end_of_input(&mut self) -> LexerResult {
        debug_assert_eq!(self.position_, self.content_.text().len());

        match self.last_token_type_ {
            TokenType::GreaterThan => self.lex_block_text(),
            TokenType::EndOfInput |
            TokenType::Outdent => {
                if self.indent_level_ > 0 {
                    self.indent_level_ -= 1;
                    self.new_token(TokenType::Outdent, None)
                } else {
                    self.new_token(TokenType::EndOfInput, None)
                }
            },
            _ => self.new_token(TokenType::EndOfLine, None),
        }
    }

    fn lex_indentation(&mut self) -> LexerResult {
        debug_assert_ne!(self.line_spaces_, self.indent_level_ * Lexer::SPACES_PER_INDENT);

        let new_level = self.line_spaces_ / Lexer::SPACES_PER_INDENT;

        if new_level < self.indent_level_ {
            debug_assert!(self.indent_level_ > 0);
            self.indent_level_ -= 1;
            return self.new_token(TokenType::Outdent, None);
        }

        if new_level > self.indent_level_ {
            self.indent_level_ += 1;
            return self.new_token(TokenType::Indent, None);
        }

        self.err("Misaligned indentation. Indents must be multiples of four spaces.")
    }

    fn lex_end_of_line(&mut self, current_ch: char) -> LexerResult {
        debug_assert!(self.is_current_char_new_line());
        debug_assert!(current_ch == '\r' || current_ch == '\n');
        debug_assert_eq!(self.position_, self.iterator_position());

        self.content_iterator_.next();
        if current_ch == '\r' {
            if let Some(&(_, '\n')) = self.content_iterator_.peek() {
                self.content_iterator_.next();
            }
        }

        self.start_new_line();
        self.new_token(TokenType::EndOfLine, None)
    }

    fn lex_word(&mut self) -> LexerResult {
        debug_assert_eq!(self.position_, self.iterator_position());
        debug_assert!(self.is_current_char_alpha());

        self.content_iterator_.next();

        while self.is_current_char_alpha() {
            self.content_iterator_.next();
        }

        let value = String::from(&self.content_.text()[self.position_..self.iterator_position()]);
        self.new_token(TokenType::Word, Some(value))
    }

    fn lex_line_text(&mut self) -> LexerResult {
        debug_assert_eq!(self.position_, self.iterator_position());

        unimplemented!()
    }

    fn lex_quoted_text(&mut self) -> LexerResult {
        debug_assert_eq!(self.position_, self.iterator_position());

        unimplemented!()
    }

    fn lex_block_text(&mut self) -> LexerResult {
        debug_assert_eq!(self.position_, self.iterator_position());

        unimplemented!()
    }

    fn parse_escape_sequence(&mut self) -> Result<char, ParserError> {
        unimplemented!()
    }

    fn get_quoted_literal(&mut self, open_quote: usize, close_quote: usize, result_length: usize, has_escapes: bool) -> &'a str {
        unimplemented!()
    }

    fn new_token(&mut self, token_type: TokenType, value: Option<String>) -> LexerResult {
        let text_start = self.position_;
        let text_end = self.iterator_position();
        self.position_ = text_end;
        self.last_token_type_ = token_type;

        match token_type {
            TokenType::Word |
            TokenType::LineText |
            TokenType::QuotedText |
            TokenType::BlockText => {
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

        Ok(Box::new(Token {
            token_type,
            content: self.content_,
            trivia_start: self.trivia_start_,
            text_start,
            text_end,
            value,
        }))
    }

    fn iterator_position(&mut self) -> usize {
        match self.content_iterator_.peek() {
            Some(&(i, _)) => i,
            None => self.content_.text().len(),
        }
    }

    fn err(&self, message: &str) -> LexerResult {
        Err(ParserError::new(self.content_, self.position_, message))
    }

    fn start_new_line(&mut self) {
        let line_start = self.iterator_position();
        self.content_.mark_line(line_start);
        self.line_start_ = line_start;
        let remaining = &self.content_.text()[line_start..]; // create a new iterator so we don't screw up the position of the main iterator
        self.line_spaces_ = remaining.chars().take_while(|&ch| ch == ' ').count();
    }

    fn is_current_char_new_line(&mut self) -> bool {
        match self.content_iterator_.peek() {
            Some(&(_, '\r')) |
            Some(&(_, '\n')) => true,
            _ => false,
        }
    }

    fn is_current_char_alpha(&mut self) -> bool {
        match self.content_iterator_.peek() {
            Some(&(_, ch)) => ch.is_ascii_alphabetic(),
            None => false,
        }
    }
}
