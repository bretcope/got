pub mod parsing;
pub mod profile;

#[macro_use]
extern crate failure;

use std::fs::File;
use std::io::Read;

fn main() {
    let mut file = match File::open("sample_profile") {
        Ok(f) => f,
        Err(err) => {
            println!("Error kind: {:?}", err.kind());
            println!("{}", err);
            return;
        }
    };

    let mut contents = String::new();
    match file.read_to_string(&mut contents) {
        Ok(_) => {},
        Err(e) => {
            println!("Error kind: {:?}", e.kind());
            println!("{}", e);
            return;
        }
    };

    println!("{}", contents);

    //println!("hey");
}