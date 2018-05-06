use super::*;

#[derive(Debug)]
pub struct Profile<'a> {
    _a: &'a str,
}

impl<'a> Profile<'a> {
    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Profile<'a>> {
        unimplemented!()
    }
}
