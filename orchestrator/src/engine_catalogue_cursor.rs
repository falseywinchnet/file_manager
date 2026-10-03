//! Predicate discrimination only. Engine retains token authentication and query
//! binding authority. No cursor is issued and no substring capability is enabled.

const SUBSTRING_FAMILY: &str = ".fm-substring-";
const SUBSTRING_V1: &str = ".fm-substring-v1.";
const MAX_SUBSTRING_CURSOR_BYTES: usize = 4096;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum CataloguePredicate {
    LegacyExact,
    SubstringV1,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum CursorError {
    UnknownRevision,
    EmptyToken,
    TooLarge,
}

/// Borrows only for this call; allocates and retains nothing. Legacy opaque
/// values retain their existing Engine validation. Only the reserved new family
/// is interpreted here; successfully classifying it does not validate its token.
pub(crate) fn catalogue_predicate(value: &str) -> Result<CataloguePredicate, CursorError> {
    if !value.starts_with(SUBSTRING_FAMILY) {
        return Ok(CataloguePredicate::LegacyExact);
    }
    if value.len() > MAX_SUBSTRING_CURSOR_BYTES {
        return Err(CursorError::TooLarge);
    }
    let token: &str = match value.strip_prefix(SUBSTRING_V1) {
        Some(token) => token,
        None => return Err(CursorError::UnknownRevision),
    };
    if token.is_empty() {
        return Err(CursorError::EmptyToken);
    }
    Ok(CataloguePredicate::SubstringV1)
}

#[cfg(test)]
mod tests {
    use super::{
        CataloguePredicate, CursorError, MAX_SUBSTRING_CURSOR_BYTES, SUBSTRING_V1,
        catalogue_predicate,
    };

    #[test]
    fn legacy_values_keep_provider_owned_validation() {
        assert_eq!(
            catalogue_predicate("old-cursor"),
            Ok(CataloguePredicate::LegacyExact)
        );
        assert_eq!(
            catalogue_predicate("eyJ2IjoxfQ"),
            Ok(CataloguePredicate::LegacyExact)
        );
        assert_eq!(catalogue_predicate(""), Ok(CataloguePredicate::LegacyExact));
    }

    #[test]
    fn reserved_revision_never_falls_through_to_exact() {
        assert_eq!(
            catalogue_predicate(".fm-substring-v1.opaque"),
            Ok(CataloguePredicate::SubstringV1)
        );
        assert_eq!(
            catalogue_predicate(".fm-substring-v1."),
            Err(CursorError::EmptyToken)
        );
        assert_eq!(
            catalogue_predicate(".fm-substring-v2.opaque"),
            Err(CursorError::UnknownRevision)
        );
        assert_eq!(
            catalogue_predicate(".fm-substring-v1"),
            Err(CursorError::UnknownRevision)
        );
    }

    #[test]
    fn wrapper_bound_counts_utf8_bytes_including_prefix() {
        let token_bytes: usize = MAX_SUBSTRING_CURSOR_BYTES - SUBSTRING_V1.len();
        let mut value: String = String::from(SUBSTRING_V1);
        let token: String = "x".repeat(token_bytes);
        value.push_str(&token);
        assert_eq!(
            catalogue_predicate(&value),
            Ok(CataloguePredicate::SubstringV1)
        );
        value.push('x');
        assert_eq!(catalogue_predicate(&value), Err(CursorError::TooLarge));
        value.truncate(MAX_SUBSTRING_CURSOR_BYTES - 1);
        value.push('é');
        assert_eq!(catalogue_predicate(&value), Err(CursorError::TooLarge));
    }
}
