use super::*;

#[derive(Debug)]
pub struct Path<'a> {
    pub property: &'a nodes::Property<'a>,
    pub name: &'a str,
    name_normalized_: String,
    pub raw_value: &'a str,
}

impl<'a> Path<'a> {
    pub fn build(prop: &'a nodes::Property<'a>, name: Option<&'a str>) -> ParsingBoxResult<Path<'a>> {
        unimplemented!()
    }

    pub fn name_normalized(&self) -> &str {
        &self.name_normalized_
    }
}
