use super::content;
use super::tokens::{Token, TokenType};
use unic_ucd_category::GeneralCategory;

pub type LexerResult<'a> = super::Result<Box<Token<'a>>>;

pub struct Lexer<'a> {
    pub file_content: &'a content::FileContent,
    ch_iter_: content::ContentIterator<'a>,
    leading_trivia_start_: usize,
    text_start_: usize,
    next_result_: Option<LexerResult<'a>>,
    is_initial_state_:  bool,
    is_end_state_: bool,
}

impl<'a> Lexer<'a> {
    pub fn new(file_content: &content::FileContent) -> Lexer {
        let mut lexer = Lexer {
            file_content,
            ch_iter_: file_content.iter(0),
            leading_trivia_start_: 0,
            text_start_: 0,
            next_result_: None,
            is_initial_state_: true,
            is_end_state_: false,
        };

        file_content.reset_line_markers();
        lexer.start_new_line(0);
        lexer.ch_iter_.next();

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

    pub fn advance(&'a mut self) -> LexerResult {
        let next = self.next_result_.take();
        match next {
            Some(result) => result,
            None => self.lex(),
        }
    }

    fn start_new_line(&mut self, line_start: usize) {
        self.file_content.mark_line(line_start);
    }

    fn reset_iterator(&mut self, position: usize) {
        if position != self.position() {
            self.ch_iter_ = self.file_content.iter(position);
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

    fn char_at(&self, pos: usize) -> char {
        self.file_content.text()[pos..].chars().next().unwrap()
    }

    fn is_end_of_input(&self) -> bool {
        self.ch_iter_.is_end_of_input()
    }

    fn content_slice(&self, start: usize, end: usize) -> &str {
        &self.file_content.text()[start..end]
    }

    fn error(&self, pos: usize, message: String) -> super::error::ParsingError {
        let pos_details = self.file_content.position_details(pos);
        super::error::ParsingError::new(&pos_details, message)
    }

    fn err_result<T>(&self, pos: usize, message: String) -> super::Result<T> {
        Err(self.error(pos, message))
    }

    fn err_unexpected_char<T>(&self) -> super::Result<T> {
        self.err_result(self.position(), format!("Unexpected character: {}", self.current_char()))
    }

    fn create_start_of_input(&self) -> LexerResult {
        panic!("not impl")
    }

    fn new_token(&mut self, token_type: TokenType, value: Option<String>) -> LexerResult<'a> {
        // need to save trivia start and text start because they get overwritten by consume_trivia
        let leading_trivia_start = self.leading_trivia_start_;
        let text_start = self.text_start_;
        let text_end = self.position();
        self.consume_trivia();

        Ok(Box::new(Token::new(
            token_type,
            self.file_content,
            leading_trivia_start,
            self.position(),
            text_start,
            text_end,
            value,
        )))
    }

    fn consume_trivia(&mut self) -> super::Result<()> {
        let text_end = self.position();
        let mut split = text_end; // Point where it switches from trailing to leading trivia
        let mut is_original_line = true;
        let mut current_line_has_comment = false;

        while !self.is_end_of_input() {
            let ch = self.current_char();
            if ch == '#' {
                self.consume_comment();
                current_line_has_comment = true;
            } else if ch == '\n' || ch == '\r' {
                self.handle_new_line_sequence()?;
                if is_original_line || !current_line_has_comment {
                    split = self.position();
                    is_original_line = false;
                }
                current_line_has_comment = false;
            } else if is_whitespace(ch) {
                self.next_char();
            } else {
                break;
            }
        }

        if self.is_initial_state_ && is_original_line {
            // If this is the start of the file and the first line is not simply whitespace or a
            // comment, then treat that whitespace as leading trivia for the first token instead of
            // trailing trivia on the StartOfInput token.
            self.leading_trivia_start_ = text_end;
        } else {
            self.leading_trivia_start_ = split;
        }

        self.text_start_ = self.position();

        Ok(())
    }

    fn consume_comment(&mut self) {
        debug_assert_eq!(self.current_char(), '#');

        while let Some(ch) = self.next_char() {
            if is_new_line_char(ch) {
                break;
            }
        }
    }

    fn handle_new_line_sequence(&mut self) -> super::Result<()> {
        let ch = self.current_char();
        if ch == '\r' {
            // make sure the CR is immediately followed by LF
            let cr_pos = self.position();
            match self.next_char() {
                Some('\n') => Ok(()),
                _ => self.err_result(
                    cr_pos,
                    String::from("Carriage return (CR) was not followed by a line feed (LF)")),
            }?
        }

        debug_assert_eq!(self.current_char(), '\n', "Expected LF");
        self.next_char();
        self.file_content.mark_line(self.position());
        Ok(())
    }

    fn lex(&mut self) -> LexerResult<'a> {
        if self.is_initial_state_ {
            let result = self.new_token(TokenType::StartOfInput, None);
            self.is_initial_state_ = false;
            return result;
        }

        if self.is_end_of_input() {
            if self.is_end_state_ {
                // prevent accidental infinite loop
                panic!("MOT Bug: Parser attempted to read past end of input")
            }
            self.is_end_state_ = true;
            return self.new_token(TokenType::EndOfInput, None);
        }

        // trivia should already have been consumed at this point
        let ch = self.current_char();
        match ch {
            ':' => {
                self.next_char();
                self.new_token(TokenType::Colon, None)
            },
            '[' => {
                self.next_char();
                self.new_token(TokenType::OpenBracket, None)
            },
            ']' => {
                self.next_char();
                self.new_token(TokenType::CloseBracket, None)
            },
            '@' => {
                match self.next_char() {
                    Some('[') => {
                        self.next_char();
                        self.new_token(TokenType::AtOpenBracket, None)
                    },
                    Some('=') => self.lex_literal_value(),
                    None => self.err_result(self.position(), String::from("Unexpected end of file")),
                    _ => self.err_unexpected_char(),
                }
            },
            '=' => self.lex_value(),
            _ => {
                if is_identifier_char(ch) {
                    self.lex_identifier()
                } else {
                    self.err_unexpected_char()
                }
            },
        }
    }

    fn lex_literal_value(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.char_at(self.text_start_), '@');
        debug_assert_eq!(self.current_char(), '=');

        let start = self.position() + 1;

        // figure out whether this is single-line or multi-line
        while let Some(ch) = self.next_char() {
            if !is_whitespace(ch) {
                break;
            }
        }

        if !self.is_end_of_input() {
            if is_new_line_char(self.current_char()) {
                // we ran into an end-of-line before any non-whitespace, so this is multi-line
                return self.lex_multiline_value();
            }

            // this is a single-line value, so find the end of the line
            while let Some(ch) = self.next_char() {
                if is_new_line_char(ch) {
                    break;
                }
            }
        }

        // we should now be at the end of a single-line value
        debug_assert!(self.is_end_of_input() || is_new_line_char(self.current_char()));

        let value = String::from(&self.file_content.text()[start..self.position()]);
        self.new_token(TokenType::Value, Some(value))
    }

    fn lex_multiline_value(&mut self) -> LexerResult<'a> {
        debug_assert!(is_new_line_char(self.current_char()));

        // consume current new line
        self.handle_new_line_sequence();

        // look for each subsequent line which starts with ==

        panic!("not impl")
    }

    fn lex_value(&mut self) -> LexerResult<'a> {
        debug_assert_eq!(self.current_char(), '=');

        // trim leading whitespace
        while let Some(ch) = self.next_char() {
            if !is_whitespace(ch) {
                break;
            }
        }

        let start = self.position();

        // find end of line
        if !self.is_end_of_input() && !is_new_line_char(self.current_char()) {
            while let Some(ch) = self.next_char() {
                if is_new_line_char(ch) {
                    break;
                }
            }
        }

        // trim trailing whitespace
        let untrimmed = &self.file_content.text()[start..self.position()];
        let mut end = untrimmed.len();
        for (i, ch) in untrimmed.char_indices().rev() {
            if !is_whitespace(ch) {
                break;
            }
            end = i;
        }

        let value = String::from(&untrimmed[0..end]);
        self.new_token(TokenType::Value, Some(value))
    }

    fn lex_identifier(&mut self) -> LexerResult<'a> {
        debug_assert!(is_identifier_char(self.current_char()));

        while let Some(ch) = self.next_char() {
            if !is_identifier_char(ch) {
                break;
            }
        }

        let ident = String::from(&self.file_content.text()[self.text_start_..self.position()]);
        self.new_token(TokenType::Identifier, Some(ident))
    }
}

fn is_identifier_char(ch: char) -> bool {
    if ch < 'a' {
        if ch < 'A' {
            return ch >= '0' && ch <= '9';
        }
        return ch <= 'Z' || ch == '_';
    }

    if ch <= 'z' {
        return true
    }

    if ch <= '\u{7F}' {
        return false; // we've covered all the ASCII cases already
    }

    match GeneralCategory::of(ch) {
        GeneralCategory::UppercaseLetter |
        GeneralCategory::LowercaseLetter |
        GeneralCategory::TitlecaseLetter |
        GeneralCategory::ModifierLetter |
        GeneralCategory::OtherLetter |
        GeneralCategory::LetterNumber |
        GeneralCategory::DecimalNumber |
        GeneralCategory::ConnectorPunctuation |
        GeneralCategory::NonspacingMark |
        GeneralCategory::SpacingMark |
        GeneralCategory::Format => true,
        _ => false,
    }
}

fn is_whitespace(ch: char) -> bool {
    let is_match = match ch {
        ' ' | // space
        '\u{0009}' | // HORIZONTAL TAB
        '\u{000b}' | // VERTICAL TAB
        '\u{000c}' | // FORM FEED
        '\u{0085}' | // NEXT LINE
        '\u{00a0}' // NO-BREAK SPACE
            => true,
        _ => false,
    };

    if is_match {
        return true;
    }

    match GeneralCategory::of(ch) {
        GeneralCategory::SpaceSeparator |
        GeneralCategory::LineSeparator |
        GeneralCategory::ParagraphSeparator => true,
        _ => false,
    }
}

fn is_new_line_char(ch: char) -> bool {
    ch == '\n' || ch == '\r'
}
