mod content;

use failure::Error;
pub use self::content::*;

pub struct Profile {
    _contents: Vec<FileContent>,
}

impl Profile {
    pub fn load(filename: &str) -> Result<Profile, Error> {
        let _content = FileContent::load(filename)?;

        unimplemented!()
    }
}