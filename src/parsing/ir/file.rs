use super::*;

#[derive(Debug)]
pub struct File<'a> {
    file_node_: &'a nodes::File<'a>,
    file_type_: FileType,
    aliases_: Vec<Box<Alias<'a>>>,
    includes_: Vec<Box<Include<'a>>>,
    prefixes_: Vec<Box<Prefix<'a>>>,
    repos_: Vec<Box<Repo<'a>>>,
}

impl<'a> File<'a> {
    pub fn file_node(&self) -> &'a nodes::File<'a> {
        &self.file_node_
    }

    pub fn file_type(&self) -> FileType {
        self.file_type_
    }

    pub fn aliases(&self) -> &Vec<Box<Alias<'a>>> {
        &self.aliases_
    }

    pub fn includes(&self) -> &Vec<Box<Include<'a>>> {
        &self.includes_
    }

    pub fn prefixes(&self) -> &Vec<Box<Prefix<'a>>> {
        &self.prefixes_
    }

    pub fn repos(&self) -> &Vec<Box<Repo<'a>>> {
        &self.repos_
    }

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
                keywords::ALIAS => {
                    let alias = Alias::build(prop)?;
                    aliases.push(alias);
                },
                keywords::INCLUDE => {
                    let include = Include::build(prop)?;
                    includes.push(include);
                },
                keywords::PREFIX => {
                    let prefix = Prefix::build(prop)?;
                    prefixes.push(prefix);
                },
                keywords::REPO => {
                    let repo = Repo::build(prop)?;
                    repos.push(repo);
                },
                _ => {
                    err_unexpected_property(
                        prop,
                        &[keywords::ALIAS, keywords::INCLUDE, keywords::PREFIX, keywords::REPO],
                    )?;
                    panic!("MOT Bug: err_result did not return an error.");
                },
            }
        }

        let file = Box::new(File {
            file_node_: file_node,
            file_type_: file_type,
            aliases_: aliases,
            includes_: includes,
            prefixes_: prefixes,
            repos_: repos,
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

    if let Some(_value) = prop.value_node() {
        return err_unexpected_value(prop);
    }

    if let Some(_block) = prop.block() {
        return err_unexpected_block(prop);
    }

    Ok(())
}
