use super::*;

#[derive(Debug)]
pub struct Path<'a> {
    property_: &'a nodes::Property<'a>,
    name_: &'a str,
    name_normalized_: String,
    raw_value_: &'a str,
}

impl<'a> Path<'a> {
    pub fn property(&self) -> &nodes::Property<'a> {
        &self.property_
    }

    pub fn name(&self) -> &str {
        &self.name_
    }

    pub fn name_normalized(&self) -> &str {
        &self.name_normalized_
    }

    pub fn raw_value(&self) -> &str {
        &self.raw_value_
    }

    pub fn build(prop: &'a nodes::Property<'a>, name: Option<&'a str>) -> ParsingBoxResult<Path<'a>> {
        let name_ = match name {
            Some(n) => n,
            None => keywords::DEFAULT,
        };

        let name_normalized_ = str_normalize(name_);

        let raw_value_ = match prop.value_str() {
            Some(v) => v,
            None => return err_missing_value(prop),
        };

        Ok(Box::new(Path {
            property_: prop,
            name_,
            name_normalized_,
            raw_value_,
        }))
    }
}
