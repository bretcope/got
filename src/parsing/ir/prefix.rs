use super::*;
use std::collections::HashSet;
use std::borrow::{Borrow,BorrowMut};

#[derive(Debug)]
pub struct Prefix<'a> {
    pub property: &'a nodes::Property<'a>,
    pub name: &'a str,
    name_normalized_: String,
    paths_: Vec<Box<Path<'a>>>,
    pub override_mode: OverrideMode,
}

impl<'a> Prefix<'a> {
    pub fn name_normalized(&self) -> &str {
        &self.name_normalized_
    }

    pub fn paths(&self) -> &Vec<Box<Path<'a>>> {
        &self.paths_
    }

    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Prefix<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::PREFIX);

        let name = match prop.property_name() {
            Some(n) => n,
            None => ""
        };

        let name_normalized = str_normalize(name);
        if name_normalized.is_empty() {
            return err_result(prop, format!(
                "\"{}\" property must have a non-empty name associated with it.",
                keywords::PREFIX,
            ));
        }

        let mut prefix = Box::new(Prefix {
            property: prop,
            name,
            name_normalized_: name_normalized,
            paths_: Vec::new(),
            override_mode: OverrideMode::Undefined,
        });

        if let Some(value_node) = prop.value_node() {
            let path = Path::build(prop, Some(keywords::DEFAULT))?;
            prefix.paths_.push(path);
        } else if let Some(block) = prop.block() {
            build_from_block(prefix.borrow_mut(), block)?;
        }

        assert_valid_paths(prefix.borrow())?;

        Ok(prefix)
    }
}

fn build_from_block<'a>(prefix: &mut Prefix<'a>, block: &'a nodes::PropertyBlock<'a>) -> ParsingResult<()> {
    for child_prop in block.property_list().properties() {
        match child_prop.property_type() {
            keywords::ENV => {
                let path = Path::build(child_prop, child_prop.property_name())?;
                prefix.paths_.push(path);
            },
            keywords::OVERRIDE => prefix.override_mode = build_override(child_prop, prefix.override_mode)?,
            _ => return err_result(child_prop.as_node(), format!(
                "Unknown property type \"{}\" in {} \"{}\"\n    Expected: {} or {}",
                child_prop.property_type(),
                keywords::PREFIX,
                prefix.name,
                keywords::ENV,
                keywords::OVERRIDE,
            )),
        }
    }

    Ok(())
}

fn build_override<'a>(prop: &'a nodes::Property<'a>, current_mode: OverrideMode) -> ParsingResult<OverrideMode> {
    debug_assert_eq!(prop.property_type(), keywords::OVERRIDE);

    if current_mode != OverrideMode::Undefined {
        return err_result(prop, format!("Duplicate {} property", keywords::OVERRIDE));
    }

    if let Some(name) = prop.property_name() {
        return err_result(prop, format!(
            "Unexpected name \"{}\" given to {} property",
            name,
            keywords::OVERRIDE,
        ));
    }

    match prop.value_str() {
        Some(keywords::MERGE) => Ok(OverrideMode::Merge),
        Some(keywords::REPLACE) => Ok(OverrideMode::Replace),
        Some(keywords::NONE) => Ok(OverrideMode::None),
        _ => err_result(prop, format!(
            "Value of {} must be \"{}\", \"{}\", or \"{}\"",
            keywords::OVERRIDE,
            keywords::MERGE,
            keywords::REPLACE,
            keywords::NONE,
        )),
    }
}

fn assert_valid_paths(prefix: &Prefix) -> ParsingResult<()> {
    if prefix.paths_.len() == 0 {
        return err_result(prefix.property, format!(
            "No \"{}\" properties defined in {} \"{}\"",
            keywords::ENV,
            keywords::PREFIX,
            prefix.name,
        ));
    }

    let mut set = HashSet::new();
    for path in prefix.paths() {
        let name_normalized = path.name_normalized();

        if name_normalized.is_empty() {
            return err_result(path.property, format!(
                "\"{}\" name is empty.",
                keywords::ENV,
            ));
        }

        let existed = set.insert(name_normalized);
        if existed {
            return err_result(path.property, format!(
                "Duplicate {} \"{}\"",
                keywords::ENV,
                name_normalized,
            ));
        }
    }

    Ok(())
}
