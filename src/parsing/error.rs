use super::content::PositionDetails;
use std::error::Error;
use std::fmt;

#[derive(Debug, Clone)]
pub struct ParsingError {
    display_text: String,
    message: String,
    filename: String,
    line_number: usize,
    column: usize,
    byte_offset: usize,
}

impl ParsingError {
    pub fn new(position: &PositionDetails, message: String) -> ParsingError {
        let position_line = ParsingError::fmt_position_line(&position);

        let display_text = format!("Error: {}\n{}\n", message, position_line);

        ParsingError {
            display_text,
            message,
            filename: String::from(position.file_content.filename()),
            line_number: position.line_number,
            column: position.column,
            byte_offset: position.byte_offset
        }
    }

    pub fn fmt_position_line(details: &PositionDetails) -> String {
        format!("    at \"{}\" {}:{}", details.file_content.filename(), details.line_number + 1, details.column + 1)
    }
}

impl Error for ParsingError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        None
    }
}

impl fmt::Display for ParsingError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.display_text)
    }
}
