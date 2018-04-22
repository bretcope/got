
use std::fs::File;
use std::io::Read;
use std::cell::RefCell;
use std::borrow::BorrowMut;
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

pub struct PositionDetails {
    pub line_number: usize,
    pub line_start: usize,
    pub column: usize,
}
