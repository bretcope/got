use profile;

#[derive(Fail, Debug)]
#[fail(display = "{}", display_text)]
pub struct ParsingError {
    display_text: String,
    message: String,
    filename: String,
    line_number: usize,
    column: usize,
    position: usize,
}

impl ParsingError {
    pub fn new(content: &profile::FileContent, position: usize, message: String) -> ParsingError {
        let pos_details = content.position_details(position);

        let display_text = format!("Error: {}\n    at \"{}\" {}:{}\n",
            message,
            content.filename(),
            pos_details.line_number + 1,
            pos_details.column + 1
        );

        ParsingError {
            display_text,
            message,
            filename: String::from(content.filename()),
            line_number: pos_details.line_number,
            column: pos_details.column,
            position
        }
    }
}
