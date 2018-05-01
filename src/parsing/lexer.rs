use std;
use profile;
use super::*;

pub type LexerResult<'a> = Result<Box<Token<'a>>, ParsingError>;

pub struct Lexer<'a> {
    pub content: &'a profile::FileContent,
    ch_iter_: profile::ContentIterator<'a>,
    trivia_start_: usize,
    text_start_: usize,
    indent_level_: usize,
    line_start_: usize,
    line_spaces_: usize,
    last_token_type_: TokenType,
    next_result_: Option<LexerResult<'a>>,
}

impl<'a> Lexer<'a> {

    pub const SPACES_PER_INDENT: usize = 4;

    pub fn new(content: &'a profile::FileContent) -> Lexer<'a> {
        let mut lexer = Lexer {
            content,
            ch_iter_: content.iter(0),
            text_start_: 0,
            trivia_start_: 0,
            indent_level_: 0,
            line_start_: 0,
            line_spaces_: 0,
            last_token_type_: TokenType::StartOfInput,
            next_result_: None,
        };

        content.reset_line_markers();
        lexer.start_new_line(0);

        lexer.ch_iter_.next(); // load the first character to start
        debug_assert_eq!(lexer.ch_iter_.position(), 0, "The position of the first character should have been zero.");
        lexer
    }

    pub fn peek_type(&mut self) -> TokenType {
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

    pub fn advance(&mut self) -> LexerResult<'a> {
        let next = self.next_result_.take();
        match next {
            Some(result) => result,
            None => self.lex(),
        }
    }

    fn lex(&mut self) -> LexerResult<'a> {
        let last_type = self.last_token_type_;

        if let TokenType::EndOfInput = last_type {
            panic!("MOT Bug: Parser scanned past end of input.");
        }

        self.consume_trivia();

        if self.is_end_of_input() {
            return self.lex_end_of_input();
        }

        // these are the tokens after which we need to be concerned about an indent or outdent
        match last_type {
            TokenType::StartOfInput |
            TokenType::EndOfLine |
            TokenType::Indent |
            TokenType::Outdent => {
                if self.text_start_ != self.line_start_ + self.line_spaces_ {
                    // the whitespace consumed at the start of the line doesn't match the number of spaces at the start of the line, which means there might be
                    // a tab character.
                    for (i, wc) in self.content.text()[self.line_start_..self.text_start_].char_indices() {
                        if wc == '\t' {
                            let message = String::from("Tab character used for indentation. Four spaces must be used for indentation.");
                            return Err(ParsingError::new(self.content, self.line_start_ + i, message));
                        }
                    }
                }

                if self.indent_level_ * Lexer::SPACES_PER_INDENT != self.line_spaces_ {
                    return self.lex_indentation();
                }
            },
            _ => {},
        };

        match self.current_char() {
            ':' => {
                self.next_char();
                self.new_token(TokenType::Colon, None)
            },
            '>' => {
                self.next_char();
                self.new_token(TokenType::GreaterThan, None)
            },
            '"' => self.lex_quoted_text(),
            '\r' |
            '\n' => match last_type {
                TokenType::GreaterThan => self.lex_block_text(),
                _ => self.lex_end_of_line(),
            },
            _ => {
                if let TokenType::Colon = last_type {
                    self.lex_line_text()
                } else if is_alpha(self.current_char()) {
                    self.lex_word()
                } else {
                    self.err_result(format!("Unexpected character `{}`.", self.current_char()))
                }
            }
        }
    }

    fn consume_trivia(&mut self) {
        self.trivia_start_ = self.position();
        let mut is_comment = false;

        while !self.is_end_of_input() {
            match self.current_char() {
                ' ' |
                '\t' => {
                    self.next_char();
                },
                '\r' |
                '\n' => {
                    // The lexer isn't supposed to emit an EndOfLine token immediately after a StartOfInput, EndOfLine, or Outdent token.
                    match self.last_token_type_ {
                        TokenType::EndOfLine |
                        TokenType::StartOfInput |
                        TokenType::Outdent => {
                            let first_ch = self.current_char();
                            self.next_char();
                            if first_ch == '\r' && self.current_char() == '\n' {
                                self.next_char();
                            }

                            let pos = self.position(); // can remove this temp when non-lexical lifetimes lands
                            self.start_new_line(pos);
                            is_comment = false;
                        },
                        _ => break, // last token wasn't a StartOfInput, EndOfLine, or Outdent, so this new line is significant (don't consume it as trivia).
                    }
                },
                '#' => {
                    self.next_char();
                    is_comment = true;
                },
                _ => {
                    if is_comment {
                        self.next_char();
                    } else {
                        break;
                    }
                }
            };
        }

        self.text_start_ = self.position();
    }

    fn lex_end_of_input(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.text_start_, self.content.text().len());

        match self.last_token_type_ {
            TokenType::GreaterThan => self.lex_block_text(),
            TokenType::EndOfLine |
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

    fn lex_indentation(&mut self) -> LexerResult<'a> {
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

        let message = String::from("Misaligned indentation. Indents must be multiples of four spaces.");
        self.err_result(message)
    }

    fn lex_end_of_line(&mut self) -> LexerResult<'a> {
        debug_assert!(is_new_line(self.current_char()));

        let first_ch = self.current_char();
        self.next_char();
        if first_ch == '\r' && self.current_char() == '\n' {
            self.next_char();
        }

        let pos = self.position(); // can remove this temp when non-lexical lifetimes lands
        self.start_new_line(pos);
        self.new_token(TokenType::EndOfLine, None)
    }

    fn lex_word(&mut self) -> LexerResult<'a> {
        debug_assert!(is_alpha(self.current_char()));

        self.next_char();

        while is_alpha(self.current_char()) {
            self.next_char();
        }

        let value = String::from(&self.content.text()[self.text_start_..self.position()]);
        self.new_token(TokenType::Word, Some(value))
    }

    fn lex_line_text(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.position(), self.text_start_);

        let mut trailing_spaces = 0;
        self.next_char();

        while !self.is_end_of_input() {
            let ch = self.current_char();

            if is_new_line(ch) || ch == '#' {
                break;
            }

            if ch == ' ' || ch == '\t' {
                trailing_spaces += 1;
            } else {
                trailing_spaces = 0;
            }

            self.next_char();
        }

        let start = self.text_start_;
        let end = self.position() - trailing_spaces;
        let value = String::from(&self.content.text()[start..end]);

        if trailing_spaces > 0 {
            // reset the iterator back to the "end" position because we'd prefer for the trailing
            // whitespace to be consumed as trivia rather than text in a token.
            self.reset_iterator(end);
        }

        self.new_token(TokenType::LineText, Some(value))
    }

    fn lex_quoted_text(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.current_char(), '"');

        self.next_char();
        let mut value = String::new();
        let mut copyable_start = self.position();
        while !self.is_end_of_input() {
            let ch = self.current_char();

            if is_new_line(ch) {
                break;
            }

            if ch != '"' && ch != '\\' {
                self.next_char();
                continue;
            }

            // copy any literal text that we can
            let pos = self.position();
            if pos > copyable_start {
                value.push_str(&self.content.text()[copyable_start..pos]);
            }

            if ch == '"' {
                self.next_char(); // consume the close quote
                return self.new_token(TokenType::QuotedText, Some(value));
            }

            let escape_char = self.parse_escape_sequence()?;
            value.push(escape_char);

            copyable_start = self.position();
        }

        self.err_result(String::from("Unterminated quoted-string."))
    }

    fn parse_escape_sequence(&mut self) -> Result<char, ParsingError> {
        debug_assert_eq!(self.current_char(), '\\');

        let esc_pos = self.position();
        self.next_char(); // consume backslash

        if !self.is_end_of_input() {
            let control_ch = self.current_char();
            self.next_char(); // go ahead and consume the control character
            match control_ch {
                '\'' |
                '"' |
                '?' |
                '\\' => return Ok(control_ch),
                'n' => return Ok('\n'),
                'r' => return Ok('\r'),
                't' => return Ok('\t'),
                'u' => {
                    let mut digit_count = 0;
                    let mut code_point = 0u32;
                    while digit_count < 6 && !self.is_end_of_input() {
                        let mut digit_ch = self.current_char() as u32;

                        const D0: u32 = '0' as u32;
                        const D9: u32 = '9' as u32;
                        const DA: u32 = 'A' as u32;
                        const DZ: u32 = 'Z' as u32;

                        if digit_ch >= D0 && digit_ch <= D9 {
                            code_point = (code_point << 4) | (digit_ch - 0x30);
                            digit_count += 1;
                            self.next_char();
                            continue;
                        }

                        digit_ch = digit_ch & !0x20_u32; // to uppercase
                        if digit_ch >= DA && digit_ch <= DZ {
                            code_point = (code_point << 4) | (digit_ch - DA + 10);
                            digit_count += 1;
                            self.next_char();
                            continue;
                        }

                        break; // not a hex digit
                    }

                    if digit_count > 0 {
                        if let Some(esc_char) = std::char::from_u32(code_point) {
                            return Ok(esc_char);
                        }
                    }
                }
                _ => {},
            }
        }

        let message = format!("Invalid escape sequence `{}`.", &self.content.text()[esc_pos..self.position()]);
        Err(ParsingError::new(self.content, esc_pos, message))
    }

    fn lex_block_text(&mut self) -> LexerResult<'a> {
        debug_assert!(self.is_end_of_input() || is_new_line(self.current_char()));

        let required_spaces = (self.indent_level_ + 1) * Lexer::SPACES_PER_INDENT;
        let mut value = String::new();
        let mut end = self.position();

        while !self.is_end_of_input() {
            // check if the next line is part of the block text
            // start by advancing past the new line
            let leading_new_line_start = self.position();
            if self.current_char() == '\r' {
                self.next_char();
                if !self.is_end_of_input() && self.current_char() == '\n' {
                    self.next_char();
                }
            } else {
                debug_assert!(self.current_char() == '\n');
                self.next_char();
            }

            let line_start = self.position();
            let mut leading_spaces = 0;
            while !self.is_end_of_input() && self.current_char() == ' ' {
                leading_spaces += 1;
                self.next_char();
            }

            let padding = std::cmp::min(required_spaces, leading_spaces);

            if padding < required_spaces
                && !self.is_end_of_input()
                && !is_new_line(self.current_char()) {
                // We didn't reach the required number of spaces, and we're not at the end of the line.
                // This means "end" already points to the end of the block text.
                break;
            }

            // Now we're sure the previous new-line was part of this block text, so let's consume it.
            self.start_new_line(line_start);

            // if this is not the very first new line, copy the new-line sequence to the string
            if value.len() > 0 {
                value.push_str(&self.content.text()[leading_new_line_start..line_start]);
            }

            // find the end of this line so we can start over
            while !self.is_end_of_input() && !is_new_line(self.current_char()) {
                self.next_char();
            }

            // add the line's content to the value
            value.push_str(self.content_slice(line_start + padding, self.position()));

            end = self.position();
        }

        self.reset_iterator(end);
        self.new_token(TokenType::BlockText, Some(value))
    }

    fn new_token(&mut self, token_type: TokenType, value: Option<String>) -> LexerResult<'a> {
        debug_assert!(token_type != TokenType::EndOfLine || self.last_token_type_ != TokenType::EndOfLine, "Two EndOfLine tokens in a row");

        let text_start = self.text_start_;
        let text_end = self.position();
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
            content: self.content,
            trivia_start: self.trivia_start_,
            text_start,
            text_end,
            value,
        }))
    }

    fn err_result(&self, message: String) -> LexerResult<'a> {
        Err(ParsingError::new(self.content, self.text_start_, message))
    }

    fn start_new_line(&mut self, line_start: usize) {
        self.content.mark_line(line_start);
        self.line_start_ = line_start;

        // create a new iterator so we don't screw up the position of the main iterator
        let remaining = &self.content.text()[line_start..];
        self.line_spaces_ = remaining.chars().take_while(|&ch| ch == ' ').count();
    }

    fn reset_iterator(&mut self, position: usize) {
        if position != self.position() {
            self.ch_iter_ = self.content.iter(position);
            self.next_char();
        }
    }

    fn next_char(&mut self) -> Option<char> {
        self.ch_iter_.next()
    }

    fn position(&self) -> usize {
        self.ch_iter_.position()
    }

    fn current_char(&self) -> char {
        self.ch_iter_.current_char()
    }

    fn is_end_of_input(&self) -> bool {
        self.ch_iter_.is_end_of_input()
    }

    fn content_slice(&self, start: usize, end: usize) -> &'a str {
        &self.content.text()[start..end]
    }
}

fn is_new_line(ch: char) -> bool {
    match ch {
        '\r' |
        '\n' => true,
        _ => false,
    }
}

fn is_alpha(ch: char) -> bool {
    ch.is_ascii_alphabetic()
}
