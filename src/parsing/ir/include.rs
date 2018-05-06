use super::*;

#[derive(Debug)]
pub struct Include<'a> {
    _a: &'a str,
}

impl<'a> Include<'a> {
    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Include<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::INCLUDE);
        unimplemented!()
    }
}
