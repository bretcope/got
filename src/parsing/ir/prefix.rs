use super::*;
use std::collections::HashSet;
use std::borrow::{Borrow,BorrowMut};

#[derive(Debug)]
pub struct Prefix<'a> {
    property_: &'a nodes::Property<'a>,
    name_: &'a str,
    name_normalized_: String,
    paths_: Vec<Box<Path<'a>>>,
    override_mode_: OverrideMode,
}

impl<'a> Prefix<'a> {
    pub fn property(&self) -> &nodes::Property<'a> {
        &self.property_
    }

    pub fn name(&self) -> &str {
        &self.name_
    }

    pub fn name_normalized(&self) -> &str {
        &self.name_normalized_
    }

    pub fn paths(&self) -> &Vec<Box<Path<'a>>> {
        &self.paths_
    }

    pub fn override_mode(&self) -> OverrideMode {
        self.override_mode_
    }

    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Prefix<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::PREFIX);

        let name = match prop.property_name() {
            Some(n) => n,
            None => ""
        };

        let name_normalized = str_normalize(name);
        if name_normalized.is_empty() {
            return err_missing_name(prop);
        }

        let mut prefix = Box::new(Prefix {
            property_: prop,
            name_: name,
            name_normalized_: name_normalized,
            paths_: Vec::new(),
            override_mode_: OverrideMode::Undefined,
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
            keywords::OVERRIDE => prefix.override_mode_ = build_override(child_prop, prefix.override_mode_)?,
            _ => return err_unexpected_property(
                child_prop,
                &[keywords::ENV, keywords::OVERRIDE]
            ),
        }
    }

    Ok(())
}

fn assert_valid_paths(prefix: &Prefix) -> ParsingResult<()> {
    if prefix.paths_.len() == 0 {
        return err_missing_child_property(prefix.property_, keywords::ENV);
    }

    let mut set = HashSet::new();
    for path in prefix.paths() {
        let name_normalized = path.name_normalized();

        if name_normalized.is_empty() {
            return err_missing_name(path.property());
        }

        let existed = set.insert(name_normalized);
        if existed {
            return err_duplicate_property_name(path.property());
        }
    }

    Ok(())
}
