use super::*;

#[derive(Debug)]
pub struct Alias<'a> {
    property_: &'a nodes::Property<'a>,
    name_: &'a str,
    name_normalized_: String,
    value_: &'a str,
}

impl<'a> Alias<'a> {
    pub fn property(&self) -> &nodes::Property<'a> {
        &self.property_
    }

    pub fn name(&self) -> &str {
        &self.name_
    }

    pub fn name_normalized(&self) -> &str {
        &self.name_normalized_
    }

    pub fn value(&self) -> &str {
        &self.value_
    }

    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Alias<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::ALIAS);

        let name = match prop.property_name() {
            Some(n) => n,
            None => "",
        };

        let name_normalized = str_normalize(name);
        if name_normalized.is_empty() {
            return err_missing_name(prop);
        }

        if let Some(_block) = prop.block() {
            return err_unexpected_block(prop);
        }

        let value = match prop.value_str() {
            Some(v) => v,
            None => return err_missing_value(prop),
        };

        Ok(Box::new(Alias {
            property_: prop,
            name_: name,
            name_normalized_: name_normalized,
            value_: value,
        }))
    }
}
