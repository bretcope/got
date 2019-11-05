use std::{fmt, fs, io, path};
use yaml_rust::{YamlLoader, YamlEmitter, Yaml};
use std::collections::HashMap;

pub type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;

pub struct MotConfig {
    pub directories: HashMap<String, MotDirectory>,
}

pub struct MotDirectory {
    pub name: String,
    pub path: String,
}

pub fn load_configuration() -> Result<MotConfig> {
    let filename = get_default_config_filename()?;
    load_conf_file(&filename)
}

fn load_conf_file(filename: &path::PathBuf) -> Result<MotConfig> {
    let yml = read_file(filename)?;
    let docs = YamlLoader::load_from_str(&yml)?;
    let doc = &docs[0];

    let mut directories = HashMap::new();
    if let Some(yml_dirs) = doc["directories"].as_hash() {
        for (key, value) in yml_dirs.iter() {
            if let Some(dir) = read_yml_directory(key, value) {
                let name = key.as_str().unwrap().to_string();
                directories.insert(name, dir);
            }
        }
    }

    Ok(MotConfig{directories})
}

fn read_yml_directory(key: &Yaml, value: &Yaml) -> Option<MotDirectory> {
    let name = key.as_str().unwrap();
    let path = value["path"].as_str().unwrap();
    Some(MotDirectory{name: String::from(name), path: String::from(path)})
}

fn read_file(filename: &path::PathBuf) -> Result<String> {
    match fs::read_to_string(&filename) {
        Ok(contents) => Ok(contents),
        Err(e) => match e.kind() {
            io::ErrorKind::NotFound => err(format!("File not found: {}", filename.display())),
            io::ErrorKind::PermissionDenied => err(format!("Permission denied: {}", filename.display())),
            _ => Err(Box::new(e))
        }
    }
}

fn get_default_config_filename() -> Result<path::PathBuf> {
    let mut home = get_home_dir()?;
    home.push(".mot");
    home.push("mot.yml");
    Ok(home)
}

fn get_home_dir() -> Result<path::PathBuf> {
    match dirs::home_dir() {
        Some(dir) => Ok(dir),
        None => err(String::from("No home directory specified")),
    }
}

fn err<T>(message: String) -> Result<T> {
    return Err(Box::new(ConfigError{message}));
}

#[derive(Debug)]
pub struct ConfigError {
    message: String,
}

impl fmt::Display for ConfigError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.message)
    }
}

impl std::error::Error for ConfigError {
    fn description(&self) -> &str {
        &self.message
    }
}