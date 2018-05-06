use super::*;

#[derive(Debug)]
pub struct File<'a> {
    pub file_node: &'a nodes::File<'a>,
    pub file_type: FileType,
    pub aliases: Vec<Box<Alias<'a>>>,
    pub includes: Vec<Box<Include<'a>>>,
    pub prefixes: Vec<Box<Prefix<'a>>>,
    pub repos: Vec<Box<Repo<'a>>>,
}

impl<'a> File<'a> {
    pub fn build(file_node: &'a nodes::File<'a>, file_type: FileType) -> ParsingBoxResult<File<'a>> {
        let properties = file_node.property_list().properties();
        debug_assert_ne!(properties.len(), 0, "PropertyList did not have any properties.");

        validate_header(&properties[0], file_type)?;

        let mut aliases = Vec::new();
        let mut includes = Vec::new();
        let mut prefixes = Vec::new();
        let mut repos = Vec::new();

        for prop in properties.iter().skip(1) {
            let prop_type = prop.property_type();
            match prop_type {
                keywords::REPO => {
                    let repo = Repo::build(prop)?;
                    repos.push(repo);
                },
                keywords::PREFIX => {
                    let prefix = Prefix::build(prop)?;
                    prefixes.push(prefix);
                },
                keywords::INCLUDE => {
                    let include = Include::build(prop)?;
                    includes.push(include);
                },
                keywords::ALIAS => {
                    let alias = Alias::build(prop)?;
                    aliases.push(alias);
                },
                _ => {
                    err_result(prop.as_node(), format!("Unknown property \"{}\"", prop_type))?;
                    panic!("MOT Bug: err_result did not return an error.");
                },
            }
        }

        let file = Box::new(File {
            file_node,
            file_type,
            aliases,
            includes,
            prefixes,
            repos,
        });

        file.validate_identifiers()?;

        Ok(file)
    }

    fn validate_identifiers(&self) -> ParsingResult<()> {
        unimplemented!()
    }
}

fn validate_header<'a>(prop: &'a nodes::Property<'a>, file_type: FileType) -> ParsingResult<()> {
    let prop_type = prop.property_type();

    match file_type {
        FileType::Profile => {
            if prop_type != keywords::PROFILE {
                return err_result(prop, format!(
                    "Your main MOT profile needs to be declared with the \"{}\" property at the beginning of the file.",
                    keywords::PROFILE,
                ));
            }
        },
        FileType::Resource => {
            if prop_type != keywords::RESOURCE {
                return err_result(prop, format!(
                    "Resource files included by your profile must be declared with the \"{}\" property at the beginning of the resource file.",
                    keywords::RESOURCE,
                ));
            }
        },
    };

    if let Some(value) = prop.value_node() {
        return err_result(value, format!("\"{}\" property cannot have a value.", prop_type));
    }

    if let Some(block) = prop.block() {
        return err_result(block, format!("\"{}\" property cannot have child properties.", prop_type));
    }

    Ok(())
}
