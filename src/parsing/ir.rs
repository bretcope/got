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
pub struct Path<'a> {
    name_: &'a str,
    raw_value_: &'a str,
}

#[derive(Debug)]
pub struct Prefix<'a> {
    property_node_: &'a nodes::Property<'a>,
    name_: &'a str,
    path_by_environment_: HashMap<String, Path<'a>>,
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
                    err_result(prop.as_node(), format!("Unknown property \"{}\"", prop_type))?;
                    panic!("MOT Bug: err_result did not return an error.");
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

fn add_alias<'a>(prop: &'a nodes::Property<'a>, _by_name: &mut HashMap<String, Alias<'a>>) -> EmptyResult {
    debug_assert_eq!(prop.property_type(), keywords::ALIAS);
    unimplemented!()
}

fn add_include<'a>(prop: &'a nodes::Property<'a>, _includes: &mut Vec<Include<'a>>) -> EmptyResult {
    debug_assert_eq!(prop.property_type(), keywords::INCLUDE);
    unimplemented!()
}

fn add_prefix<'a>(prop: &'a nodes::Property<'a>, by_name: &mut HashMap<String, Prefix<'a>>) -> EmptyResult {
    debug_assert_eq!(prop.property_type(), keywords::PREFIX);

    let prefix_name = match prop.property_name() {
        Some(n) => n,
        None => {
            return err_result(prop, format!(
                "\"{}\" property must have a name associated with it.",
                keywords::PREFIX,
            ));
        }
    };

    let prefix_normalized = str_normalize(prefix_name);

    if let Some(existing) = by_name.get(&prefix_normalized) {
        let orig_at = ParsingError::fmt_position_line(&existing.property_node_.position_details());
        return err_result(prop, format!(
            "Duplicate {} \"{}\".\n{}",
            keywords::PREFIX,
            prefix_name,
            orig_at,
        ));
    }

    let mut ir = Prefix {
        property_node_: prop,
        name_: prefix_name,
        path_by_environment_: HashMap::new(),
        override_mode_: OverrideMode::Undefined,
    };

    if let Some(value_node) = prop.value_node() {
        let path = Path { name_: keywords::DEFAULT, raw_value_: value_node.value() };
        ir.path_by_environment_.insert(str_normalize(keywords::DEFAULT), path);
    } else if let Some(block) = prop.block() {
        for child_prop in block.property_list().properties() {
            match child_prop.property_type() {
                keywords::ENV => add_prefix_env(&mut ir, child_prop)?,
                keywords::OVERRIDE => add_prefix_override(&mut ir, child_prop)?,
                _ => return err_result(child_prop.as_node(), format!(
                    "Unknown property type \"{}\" in {} \"{}\"\n    Expected: {} or {}",
                    child_prop.property_type(),
                    keywords::PREFIX,
                    prefix_name,
                    keywords::ENV,
                    keywords::OVERRIDE,
                )),
            }
        }
    }

    if ir.path_by_environment_.len() == 0 {
        return err_result(prop, format!(
            "No \"{}\" properties defined in {} \"{}\"",
            keywords::ENV,
            keywords::PREFIX,
            prefix_name,
        ));
    }

    by_name.insert(prefix_normalized, ir);
    Ok(())
}

fn add_prefix_env<'a>(ir: &mut Prefix<'a>, prop: &'a nodes::Property<'a>) -> EmptyResult {
    let env_name = match prop.property_name() {
        Some(n) => n,
        None => {
            return err_result(prop.as_node(), format!(
                "Environment name is empty in {} \"{}\"",
                keywords::PREFIX,
                ir.name_,
            ));
        },
    };

    let env_normalized = str_normalize(env_name);
    if let Some(_) = ir.path_by_environment_.get(&env_normalized) {
        return err_result(prop.as_node(), format!(
            "Duplicate environment \"{}\" in {} \"{}\"",
            env_name,
            keywords::PREFIX,
            ir.name_,
        ));
    }

    let env_path = match prop.value_node() {
        Some(value_prop) => value_prop.value(),
        None => "",
    };

    if str_is_whitespace(env_path) {
        return err_result(prop.as_node(), format!(
            "Value of {} \"{}\" is missing or empty in {} \"{}\"",
            keywords::ENV,
            env_name,
            keywords::PREFIX,
            ir.name_,
        ));
    }

    let path = Path { name_: env_name, raw_value_: env_path };
    ir.path_by_environment_.insert(env_normalized, path);

    Ok(())
}

fn add_prefix_override<'a>(ir: &mut Prefix<'a>, prop: &'a nodes::Property<'a>) -> EmptyResult {
    if ir.override_mode_ != OverrideMode::Undefined {
        return err_result(prop.as_node(), format!(
            "Multiple {} properties in {} \"{}\"",
            keywords::OVERRIDE,
            keywords::PREFIX,
            ir.name_,
        ));
    }

    if let Some(_) = prop.property_name() {
        return err_result(prop.as_node(), format!(
            "Expected a colon (:) after \"{}\" in {} \"{}\"",
            keywords::OVERRIDE,
            keywords::PREFIX,
            ir.name_,
        ));
    }

    ir.override_mode_ = match prop.value_str() {
        Some(keywords::MERGE) => OverrideMode::Merge,
        Some(keywords::REPLACE) => OverrideMode::Replace,
        Some(keywords::NONE) => OverrideMode::None,
        _ => return err_result(prop.as_node(), format!(
            "Value of {} must be \"{}\", \"{}\", or \"{}\"",
            keywords::OVERRIDE,
            keywords::MERGE,
            keywords::REPLACE,
            keywords::NONE,
        )),
    };

    Ok(())
}

fn add_repo<'a>(prop: &'a nodes::Property<'a>, _by_name: &mut HashMap<String, Repo<'a>>) -> EmptyResult {
    debug_assert_eq!(prop.property_type(), keywords::REPO);
    unimplemented!()
}

fn err_result<'a, T>(node: &'a nodes::Node<'a>, message: String) -> Result<T, ParsingError> {
    let pos = node.position_details();
    Err(ParsingError::new(&pos, message))
}
