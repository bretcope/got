use std;
use profile;
use super::*;

pub type LexerResult<'a> = Result<Box<Token<'a>>, ParserError>;

pub struct Lexer<'a> {
    content_: &'a profile::FileContent,
    ch_iter: profile::ContentIterator<'a>,
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
            content_: content,
            ch_iter: content.iter(0),
            text_start_: 0,
            trivia_start_: 0,
            indent_level_: 0,
            line_start_: 0,
            line_spaces_: 0,
            last_token_type_: TokenType::StartOfInput,
            next_result_: None,
        };

        lexer.ch_iter.next(); // load the first character to start
        debug_assert_eq!(lexer.ch_iter.position(), 0, "The position of the first character should have been zero.");
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

        if self.ch_iter.is_end_of_input() {
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
                    for (i, wc) in self.content_.text()[self.line_start_..self.text_start_].char_indices() {
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

        match self.ch_iter.current_char() {
            ':' => {
                self.ch_iter.next();
                self.new_token(TokenType::Colon, None)
            },
            '>' => {
                self.ch_iter.next();
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
                } else if is_alpha(self.ch_iter.current_char()) {
                    self.lex_word()
                } else {
                    self.err_result("Unexpected character.")
                }
            }
        }
    }

    fn consume_trivia(&mut self) {
        self.trivia_start_ = self.ch_iter.position();
        let mut is_comment = false;

        while !self.ch_iter.is_end_of_input() {
            match self.ch_iter.current_char() {
                ' ' |
                '\t' => {
                    self.ch_iter.next();
                },
                '\r' |
                '\n' => {
                    // The lexer isn't supposed to emit an EndOfLine token immediately after a StartOfInput, EndOfLine, or Outdent token.
                    match self.last_token_type_ {
                        TokenType::EndOfInput |
                        TokenType::StartOfInput |
                        TokenType::Outdent => {
                            let first_ch = self.ch_iter.current_char();
                            self.ch_iter.next();
                            if first_ch == '\r' && self.ch_iter.current_char() == '\n' {
                                self.ch_iter.next();
                            }

                            self.start_new_line();
                            is_comment = false;
                        },
                        _ => break, // last token wasn't a StartOfInput, EndOfLine, or Outdent, so this new line is significant (don't consume it as trivia).
                    }
                },
                '#' => {
                    self.ch_iter.next();
                    is_comment = true;
                },
                _ => {
                    if is_comment {
                        self.ch_iter.next();
                    } else {
                        break;
                    }
                }
            };
        }

        self.text_start_ = self.ch_iter.position();
    }

    fn lex_end_of_input(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.text_start_, self.content_.text().len());

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

        self.err_result("Misaligned indentation. Indents must be multiples of four spaces.")
    }

    fn lex_end_of_line(&mut self) -> LexerResult<'a> {
        debug_assert!(is_new_line(self.ch_iter.current_char()));

        let first_ch = self.ch_iter.current_char();
        self.ch_iter.next();
        if first_ch == '\r' && self.ch_iter.current_char() == '\n' {
            self.ch_iter.next();
        }

        self.start_new_line();
        self.new_token(TokenType::EndOfLine, None)
    }

    fn lex_word(&mut self) -> LexerResult<'a> {
        debug_assert!(is_alpha(self.ch_iter.current_char()));

        self.ch_iter.next();

        while is_alpha(self.ch_iter.current_char()) {
            self.ch_iter.next();
        }

        let value = String::from(&self.content_.text()[self.text_start_..self.ch_iter.position()]);
        self.new_token(TokenType::Word, Some(value))
    }

    fn lex_line_text(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.ch_iter.position(), self.text_start_);

        let mut trailing_spaces = 0;
        self.ch_iter.next();

        while !self.ch_iter.is_end_of_input() {
            let ch = self.ch_iter.current_char();

            if is_new_line(ch) || ch == '#' {
                break;
            }

            if ch != ' ' && ch != '\t' {
                trailing_spaces += 1;
            } else {
                trailing_spaces = 0;
            }

            self.ch_iter.next();
        }

        let start = self.text_start_;
        let end = self.ch_iter.position() - trailing_spaces;
        let value = String::from(&self.content_.text()[start..end]);

        if trailing_spaces > 0 {
            // reset the iterator back to the "end" position because we'd prefer for the trailing
            // whitespace to be consumed as trivia rather than text in a token.
            self.ch_iter = self.content_.iter(end);
        }

        self.new_token(TokenType::LineText, Some(value))
    }

    fn lex_quoted_text(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.ch_iter.current_char(), '"');

        self.ch_iter.next();
        let mut value = String::new();
        let mut copyable_start = self.ch_iter.position();
        while !self.ch_iter.is_end_of_input() {
            let ch = self.ch_iter.current_char();

            if is_new_line(ch) {
                break;
            }

            if ch != '"' && ch != '\\' {
                self.ch_iter.next();
                continue;
            }

            // copy any literal text that we can
            let pos = self.ch_iter.position();
            if pos > copyable_start {
                value.push_str(&self.content_.text()[copyable_start..pos]);
            }

            if ch == '"' {
                self.ch_iter.next(); // consume the close quote
                return self.new_token(TokenType::QuotedText, Some(value));
            }

            let escape_char = self.parse_escape_sequence()?;
            value.push(escape_char);

            copyable_start = self.ch_iter.position();
        }

        self.err_result("Unterminated quoted-string.")
    }

    fn parse_escape_sequence(&mut self) -> Result<char, ParserError> {
        debug_assert_eq!(self.ch_iter.current_char(), '\\');

        let esc_pos = self.ch_iter.position();
        self.ch_iter.next(); // consume backslash

        if !self.ch_iter.is_end_of_input() {
            let control_ch = self.ch_iter.current_char();
            self.ch_iter.next(); // go ahead and consume the control character
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
                    while digit_count < 6 && !self.ch_iter.is_end_of_input() {
                        let mut digit_ch = self.ch_iter.current_char() as u32;

                        const D0: u32 = '0' as u32;
                        const D9: u32 = '9' as u32;
                        const DA: u32 = 'A' as u32;
                        const DZ: u32 = 'Z' as u32;

                        if digit_ch >= D0 && digit_ch <= D9 {
                            code_point = (code_point << 4) | (digit_ch - 0x30);
                            continue;
                        }

                        digit_ch = digit_ch & !0x20_u32; // to uppercase
                        if digit_ch >= DA && digit_ch <= DZ {
                            code_point = (code_point << 4) | (digit_ch - DA + 10);
                            continue;
                        }

                        break; // not a hex digit
                    }

                    if digit_count > 0 && code_point <= std::char::MAX as u32 {
                        unsafe {
                            return Ok(std::mem::transmute::<u32, char>(code_point));
                        }
                    }
                }
                _ => {},
            }
        }

        Err(ParserError::new(self.content_, esc_pos, "Invalid escape sequence."))
    }

    fn lex_block_text(&mut self) -> LexerResult<'a> {
        //

        unimplemented!()
    }

    fn new_token(&mut self, token_type: TokenType, value: Option<String>) -> LexerResult<'a> {
        let text_start = self.text_start_;
        let text_end = self.ch_iter.position();
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

    fn err_result(&self, message: &str) -> LexerResult<'a> {
        Err(ParserError::new(self.content_, self.text_start_, message))
    }

    fn start_new_line(&mut self) {
        let line_start = self.ch_iter.position();
        self.content_.mark_line(line_start);
        self.line_start_ = line_start;

        // create a new iterator so we don't screw up the position of the main iterator
        let remaining = &self.content_.text()[line_start..];
        self.line_spaces_ = remaining.chars().take_while(|&ch| ch == ' ').count();
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
