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
use super::nodes::Node;

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
