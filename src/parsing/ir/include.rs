use super::*;

#[derive(Debug)]
pub struct Include<'a> {
    property_: &'a nodes::Property<'a>,
    name_: Option<&'a str>,
    file_: &'a str,
    renames_: Vec<Rename<'a>>,
}

#[derive(Debug)]
pub struct Rename<'a> {
    pub from_pattern: &'a str,
    pub to_pattern: &'a str,
}

impl<'a> Include<'a> {
    pub fn property(&self) -> &'a nodes::Property<'a> {
        &self.property_
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name_
    }

    pub fn file(&self) -> &'a str {
        &self.file_
    }

    pub fn renames(&self) -> &Vec<Rename<'a>> {
        &self.renames_
    }

    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Include<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::INCLUDE);
        unimplemented!("Includes are not implemented yet.")
    }
}
