use std::collections::HashMap;
use super::*;
use super::nodes::Node;

type EmptyResult = Result<(), ParsingError>;

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub enum FileType {
    Profile,
    Resource,
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub enum OverrideMode {
    Undefined,
    None,
    Replace,
    Merge,
}

#[derive(Debug)]
pub struct Profile<'a> {
    _a: &'a str,
}

#[derive(Debug)]
pub struct File<'a> {
    file_node_: &'a nodes::File<'a>,
    file_type_: FileType,
    alias_by_name_: HashMap<String, Alias<'a>>,
    includes_: Vec<Include<'a>>,
    prefix_by_name_: HashMap<String, Prefix<'a>>,
    repo_by_name_: HashMap<String, Repo<'a>>,
}

#[derive(Debug)]
pub struct Alias<'a> {
    _a: &'a str,
}

#[derive(Debug)]
pub struct Include<'a> {
    _a: &'a str,
}

#[derive(Debug)]
pub struct Prefix<'a> {
    property_node_: &'a nodes::Property<'a>,
    name_: &'a str,
    path_by_environment_: HashMap<String, &'a str>,
    override_mode_: OverrideMode,
}

#[derive(Debug)]
pub struct Repo<'a> {
    name_: &'a str,
    config_: Option<&'a str>,
    remote_by_name_: HashMap<&'a str, &'a str>,
    override_mode_: OverrideMode,
}

impl<'a> File<'a> {
    pub fn build(file_node: &'a nodes::File<'a>, file_type: FileType) -> ParserResult<File<'a>> {
        let properties = file_node.property_list().properties();
        debug_assert_ne!(properties.len(), 0, "PropertyList did not have any properties.");

        validate_header(&properties[0], file_type)?;

        let mut alias_by_name_ = HashMap::new();
        let mut includes_ = Vec::new();
        let mut prefix_by_name_ = HashMap::new();
        let mut repo_by_name_ = HashMap::new();

        for prop in properties.iter().skip(1) {
            let prop_type = prop.property_type();
            match prop_type {
                keywords::REPO => add_repo(prop, &mut repo_by_name_)?,
                keywords::PREFIX => add_prefix(prop, &mut prefix_by_name_)?,
                keywords::INCLUDE => add_include(prop, &mut includes_)?,
                keywords::ALIAS => add_alias(prop, &mut alias_by_name_)?,
                _ => {
                    let (content, position) = prop.content_position();
                    let message = format!("Unknown property \"{}\"", prop_type);
                    return Err(ParsingError::new(content, position, &message));
                },
            }
        }

        Ok(Box::new(File {
            file_node_: file_node,
            file_type_: file_type,
            alias_by_name_,
            includes_,
            prefix_by_name_,
            repo_by_name_
        }))
    }
}

fn validate_header<'a>(prop: &'a nodes::Property<'a>, file_type: FileType) -> EmptyResult {
    let prop_type = prop.property_type();

    match file_type {
        FileType::Profile => {
            if prop_type != keywords::PROFILE {
                let (content, position) = prop.content_position();
                let message = format!("Your main MOT profile needs to be declared with the \"{}\" property at the beginning of the file.", keywords::PROFILE);
                return Err(ParsingError::new(content, position, &message));
            }
        },
        FileType::Resource => {
            if prop_type != keywords::RESOURCE {
                let (content, position) = prop.content_position();
                let message = format!("Resource files included by your profile byst me declared with the \"{}\" property at the beginning of the file.", keywords::RESOURCE);
                return Err(ParsingError::new(content, position, &message));
            }
        },
    };

    if let Some(value) = prop.value() {
        let (content, position) = value.content_position();
        let message = format!("\"{}\" property cannot have a value.", prop_type);
        return Err(ParsingError::new(content, position, &message));
    }

    if let Some(block) = prop.block() {
        let (content, position) = block.content_position();
        let message = format!("\"{}\" property cannot have child properties.", prop_type);
        return Err(ParsingError::new(content, position, &message));
    }

    Ok(())
}

fn add_alias<'a>(prop: &'a nodes::Property<'a>, by_name: &mut HashMap<String, Alias<'a>>) -> EmptyResult {
    unimplemented!()
}

fn add_include<'a>(prop: &'a nodes::Property<'a>, includes: &mut Vec<Include<'a>>) -> EmptyResult {
    unimplemented!()
}

fn add_prefix<'a>(prop: &'a nodes::Property<'a>, by_name: &mut HashMap<String, Prefix<'a>>) -> EmptyResult {
    unimplemented!()
}

fn add_repo<'a>(prop: &'a nodes::Property<'a>, by_name: &mut HashMap<String, Repo<'a>>) -> EmptyResult {
    unimplemented!()
}
