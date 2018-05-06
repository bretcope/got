use profile;

#[derive(Fail, Debug)]
#[fail(display = "{}", display_text)]
pub struct ParsingError {
    display_text: String,
    message: String,
    filename: String,
    line_number: usize,
    column: usize,
    byte_offset: usize,
}

impl ParsingError {
    pub fn new(position: &profile::PositionDetails, message: String) -> ParsingError {
        let position_line = ParsingError::fmt_position_line(&position);

        let display_text = format!("Error: {}\n{}\n", message, position_line);

        ParsingError {
            display_text,
            message,
            filename: String::from(position.content.filename()),
            line_number: position.line_number,
            column: position.column,
            byte_offset: position.byte_offset
        }
    }

    pub fn fmt_position_line(details: &profile::PositionDetails) -> String {
        format!("    at \"{}\" {}:{}", details.content.filename(), details.line_number + 1, details.column + 1)
    }
}
