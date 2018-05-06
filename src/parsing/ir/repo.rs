use super::*;

#[derive(Debug)]
pub struct Repo<'a> {
    pub name: Option<&'a str>,
    pub name_normalized: Option<String>,
    pub path: Box<Path<'a>>,
    pub config: Option<&'a str>,
    pub remotes: Vec<Remote<'a>>,
    pub override_mode: OverrideMode,
}

#[derive(Debug)]
pub struct Remote<'a> {
    pub name: &'a str,
    pub url: &'a str,
}

impl<'a> Repo<'a> {
    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Repo<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::REPO);

        let name = prop.property_name();
        let name_normalized = match name {
            Some(n) => Some(str_normalize(n)),
            None => None,
        };

        if let Some(value_node) = prop.value_node() {
            return err_result(value_node, format!(
                "\"{}\" properties must have sub-properties instead of a string value.",
                keywords::REPO,
            ));
        }

        let block = match prop.block() {
            Some(b) => b,
            None => return err_result(prop, format!(
                "\"{}\" properties must have sub-properties.",
                keywords::REPO,
            )),
        };

//        let mut path = None;
//        let mut config = None;
//        let mut remotes = Vec::new();
//        let mut override_mode = OverrideMode::Undefined;

        for child_prop in block.property_list().properties() {
            //
        }

        // todo: validate we got all the properties we need

        unimplemented!()
    }
}
