
pub fn str_normalize(s: &str) -> String {
    s.to_lowercase()
}

pub fn str_is_whitespace(s: &str) -> bool {
    if s.len() == 0 {
        return true;
    }

    s.chars().all(|ch| ch.is_whitespace())
}
