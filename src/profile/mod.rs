mod content;

use failure::Error;
pub use self::content::*;

pub struct Profile {
    _contents: Vec<FileContent>,
}

impl Profile {
    fn load(filename: &str) -> Result<Profile, Error> {
        let content = FileContent::load(filename)?;

        unimplemented!()
    }
}