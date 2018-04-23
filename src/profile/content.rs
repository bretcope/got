use std::str::CharIndices;
use std::fs::File;
use std::io::Read;
use std::cell::RefCell;
use failure::Error;

#[derive(Debug)]
pub struct FileContent {
    filename_: String,
    text_: String,
    line_starts_: RefCell<Vec<usize>>,
}

impl FileContent {
    pub fn load(filename: &str) -> Result<FileContent, Error> {
        let mut file = File::open(filename)?;
        let mut text = String::new();
        file.read_to_string(&mut text)?;

        Ok(FileContent {
            filename_: String::from(filename),
            text_: text,
            line_starts_: RefCell::new(Vec::new()), // this will get updated by the lexer
        })
    }

    pub fn filename(&self) -> &str {
        &self.filename_
    }

    pub fn text(&self) -> &str {
        &self.text_
    }

    pub fn iter(&self, start: usize) -> ContentIterator {
        ContentIterator {
            iterator_: self.text_[start..].char_indices(),
            start_: start,
            end_: self.text_.len(),
            relative_position_: 0,
            current_char_: '\0',
            is_end_of_input_: false,
        }
    }

    pub fn position_details(&self, position: usize) -> PositionDetails {
        let lines = self.line_starts_.borrow();
        let count = lines.len();
        if count == 0 {
            return PositionDetails {
                line_number: 0,
                line_start: 0,
                column: 0,
            }
        }

        debug_assert_eq!(lines[0], 0);
        let mut line_start: usize;

        // perform binary search to find the line number
        let mut left: usize = 0;
        let mut right = count;
        let mut i: usize = right / 2;
        loop {
            debug_assert!(i < right);
            debug_assert!(i >= left);
            debug_assert!(right <= count);

            line_start = lines[i];

            if position >= line_start && (i + 1 == count || position < lines[i + 1]) {
                break;
            }

            if position < line_start {
                // try earlier in the list
                debug_assert_ne!(i, 0);
                right = i;
            } else {
                // try later in the list
                assert!(i + 1 < right);
                left = i + 1;
            }

            i = left + (right - left) / 2;
        }

        debug_assert!(position >= line_start);
        debug_assert!(self.text_.len() >= line_start);

        let column = self.text_[line_start..position].chars().count();

        PositionDetails {
            line_number: i,
            line_start,
            column,
        }
    }

    pub fn reset_line_markers(&self) {
        self.line_starts_.borrow_mut().clear();
    }

    pub fn mark_line(&self, position: usize) {
        self.line_starts_.borrow_mut().push(position);
    }
}

#[derive(Debug)]
pub struct PositionDetails {
    pub line_number: usize,
    pub line_start: usize,
    pub column: usize,
}

#[derive(Debug)]
pub struct ContentIterator<'a> {
    iterator_: CharIndices<'a>,
    start_: usize,
    end_: usize,
    relative_position_: usize,
    current_char_: char,
    is_end_of_input_: bool,
}

impl<'a> Iterator for ContentIterator<'a> {
    type Item = char;

    fn next(&mut self) -> Option<char> {
        match self.iterator_.next() {
            Some((i, ch)) => {
                self.relative_position_ = i;
                self.current_char_ = ch;
                Some(ch)
            },
            None => {
                self.is_end_of_input_ = true;
                self.relative_position_ = self.end_ - self.start_;
                self.current_char_ = '\0';
                None
            }
        }
    }
}

impl<'a> ContentIterator<'a> {
    pub fn is_end_of_input(&self) -> bool {
        self.is_end_of_input_
    }

    pub fn position(&self) -> usize {
        self.relative_position_ + self.start_
    }

    pub fn current_char(&self) -> char {
        self.current_char_
    }
}
