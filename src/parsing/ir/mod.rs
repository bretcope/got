mod alias;
pub use self::alias::*;
mod file;
pub use self::file::*;
mod include;
pub use self::include::*;
mod path;
pub use self::path::*;
mod prefix;
pub use self::prefix::*;
mod profile;
pub use self::profile::*;
mod repo;
pub use self::repo::*;

use super::*;

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

fn err_result<'a, T>(node: &'a nodes::Node<'a>, message: String) -> ParsingResult<T> {
    let pos = node.position_details();
    Err(ParsingError::new(&pos, message))
}

fn err_duplicate_property_type<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("Duplicate {} property.", prop.property_type()))
}

fn err_duplicate_property_name<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    let name = prop.property_name().expect("MOT Bug: missing property name.");
    err_result(prop, format!("Duplicate {} \"{}\".", prop.property_type(), name))
}

fn err_missing_child_property<'a, T>(prop: &'a nodes::Property<'a>, expected: &str) -> ParsingResult<T> {
    let quoted_name = match prop.property_name() {
        Some(name) => format!("\"{}\" ", name),
        None => "".to_string(),
    };

    err_result(prop, format!(
        "{} property {}needs a child {} property.",
        prop.property_type(),
        quoted_name,
        expected,
    ))
}

fn err_missing_name<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("{} property requires a non-empty name.", prop.property_type()))
}

fn err_missing_value<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("{} property requires a value.", prop.property_type()))
}

fn err_unexpected_block<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("{} property cannot have child properties.", prop.property_type()))
}

fn err_unexpected_value<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("{} property cannot have a string value.", prop.property_type()))
}

fn err_unexpected_name<'a, T>(prop: &'a nodes::Property<'a>) -> ParsingResult<T> {
    err_result(prop, format!("{} property cannot be named.", prop.property_type()))
}

fn err_unexpected_property<'a, T>(prop: &'a nodes::Property<'a>, expected: &[&str]) -> ParsingResult<T> {
    err_result(prop, format!(
        "Unexpected property type \"{}\".\n    Expected: {:?}",
        prop.property_type(),
        expected,
    ))
}

fn build_override<'a>(prop: &'a nodes::Property<'a>, current_mode: OverrideMode) -> ParsingResult<OverrideMode> {
    debug_assert_eq!(prop.property_type(), keywords::OVERRIDE);

    if current_mode != OverrideMode::Undefined {
        return err_duplicate_property_type(prop);
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
