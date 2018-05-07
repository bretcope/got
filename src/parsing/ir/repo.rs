use super::*;

#[derive(Debug)]
pub struct Repo<'a> {
    property_: &'a nodes::Property<'a>,
    name_: Option<&'a str>,
    name_normalized_: Option<String>,
    path_: Box<Path<'a>>,
    config_: Option<&'a str>,
    remotes_: Vec<Remote<'a>>,
    override_mode_: OverrideMode,
}

#[derive(Debug)]
pub struct Remote<'a> {
    pub name: &'a str,
    pub url: &'a str,
}

impl<'a> Repo<'a> {
    pub fn property(&self) -> &'a nodes::Property<'a> {
        &self.property_
    }

    pub fn name(&self) -> Option<&str> {
        self.name_
    }

    pub fn name_normalized(&self) -> Option<&str> {
        match &self.name_normalized_ {
            &Some(ref n) => Some(&n),
            &None => None,
        }
    }

    pub fn path(&self) -> &Path<'a> {
        &self.path_
    }

    pub fn config(&self) -> Option<&'a str> {
        self.config_
    }

    pub fn remotes(&self) -> &Vec<Remote<'a>> {
        &self.remotes_
    }

    pub fn override_mode(&self) -> OverrideMode {
        self.override_mode_
    }

    pub fn build(prop: &'a nodes::Property<'a>) -> ParsingBoxResult<Repo<'a>> {
        debug_assert_eq!(prop.property_type(), keywords::REPO);

        let name = prop.property_name();
        let name_normalized = match name {
            Some(n) => Some(str_normalize(n)),
            None => None,
        };

        if let Some(_value_node) = prop.value_node() {
            return err_unexpected_value(prop);
        }

        let mut path_opt = None;
        let mut config = None;
        let mut remotes = Vec::new();
        let mut override_mode = OverrideMode::Undefined;

        if let Some(block) = prop.block() {
            for child_prop in block.property_list().properties() {
                match child_prop.property_type() {
                    keywords::CONFIG => config = Some(build_config(child_prop, &config)?),
                    keywords::OVERRIDE => override_mode = build_override(child_prop, override_mode)?,
                    keywords::PATH => path_opt = Some(build_path(child_prop, &path_opt)?),
                    keywords::REMOTE => {
                        let remote = build_remote(child_prop)?;
                        if remotes.iter().any(|r: &Remote| r.name == remote.name) {
                            return err_duplicate_property_name(child_prop);
                        }
                        remotes.push(remote);
                    },
                    _ => return err_unexpected_property(
                        child_prop,
                        &[keywords::CONFIG, keywords::OVERRIDE, keywords::PATH, keywords::REMOTE]
                    ),
                }
            }
        }

        let path = match path_opt {
            Some(p) => p,
            None => return err_missing_child_property(prop, keywords::PATH),
        };

        if remotes.len() == 0 {
            return err_missing_child_property(prop, keywords::REMOTE);
        }

        Ok(Box::new(Repo {
            property_: prop,
            name_: name,
            name_normalized_: name_normalized,
            path_: path,
            config_: config,
            remotes_: remotes,
            override_mode_: override_mode,
        }))
    }
}

fn build_config<'a>(prop: &'a nodes::Property<'a>, existing: &Option<&str>) -> ParsingResult<&'a str> {
    debug_assert_eq!(prop.property_type(), keywords::CONFIG);

    if let &Some(_) = existing {
        return err_duplicate_property_type(prop);
    }

    if let Some(_) = prop.property_name() {
        return err_unexpected_name(prop);
    }

    match prop.value_str() {
        Some(value) => Ok(value),
        None => err_missing_value(prop),
    }
}

fn build_path<'a>(prop: &'a nodes::Property<'a>, existing: &Option<Box<Path<'a>>>) -> ParsingBoxResult<Path<'a>> {
    debug_assert_eq!(prop.property_type(), keywords::PATH);

    if let &Some(_) = existing {
        return err_duplicate_property_type(prop);
    }

    if let Some(_name) = prop.property_name() {
        return err_unexpected_name(prop);
    }

    Ok(Path::build(prop, None)?)
}

fn build_remote<'a>(prop: &'a nodes::Property<'a>) -> ParsingResult<Remote<'a>> {
    debug_assert_eq!(prop.property_type(), keywords::REMOTE);

    let name = match prop.property_name() {
        Some(n) => n,
        None => "origin",
    };

    let url = match prop.value_str() {
        Some(v) => v,
        None => return err_missing_value(prop),
    };

    Ok(Remote {
        name,
        url,
    })
}


