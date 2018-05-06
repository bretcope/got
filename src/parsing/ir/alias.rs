use super::*;

#[derive(Debug)]
pub struct Alias<'a> {
    property: &'a nodes::Property<'a>,
    name: &'a str,
    name_normalized: String,
    value: &'a str,
}

impl<'a> Alias<'a> {
    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Alias<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::ALIAS);

        let name = match prop.property_name() {
            Some(n) => n,
            None => "",
        };

        let name_normalized = str_normalize(name);
        if name_normalized.is_empty() {
            return err_result(prop, format!(
                "\"{}\" property must have a non-empty name associated with it.",
                keywords::ALIAS,
            ));
        }

        if let Some(_block) = prop.block() {
            return err_result(prop, format!(
                "\"{}\" property cannot have child properties.",
                keywords::ALIAS
            ));
        }

        let value = match prop.value_str() {
            Some(v) => v,
            None => return err_result(prop, format!(
                "Value of {} \"{}\" is missing.",
                keywords::ALIAS,
                name,
            )),
        };

        Ok(Box::new(Alias {
            property: prop,
            name,
            name_normalized,
            value,
        }))
    }
}
