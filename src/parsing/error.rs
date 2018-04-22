use profile;

#[derive(Fail, Debug)]
#[fail(display = "Error: {}", message)]
pub struct ParserError {
    message: String,
}

impl ParserError {
    pub fn new(content: &profile::FileContent, position: usize, message: &str) -> ParserError {
        // todo: real error message
        ParserError {
            message: String::from(message),
        }
    }
}