pub mod nodes;

use super::*;
use profile::FileContent;

pub type ParserResult<T> = Result<Box<T>, ParsingError>;

pub fn parse_configuration_file<'a>(content: &'a FileContent) -> ParserResult<nodes::File<'a>> {
    let mut lexer = Lexer::new(content);
    parse_file(&mut lexer)
}

/*
File
    PropertyList endOfInput
 */
fn parse_file<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::File<'a>> {
    let prop_list = parse_property_list(lexer)?;
    let end_of_input = require_token(lexer, TokenType::EndOfInput)?;

    Ok(Box::new(nodes::File {
        property_list_: prop_list,
        end_of_input_: end_of_input,
    }))
}

/*
PropertyList
    Property PropertyList
    Property
 */
fn parse_property_list<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::PropertyList<'a>> {
    let mut properties = Vec::new();
    loop {
        let prop = parse_property(lexer)?;
        properties.push(prop);

        if lexer.peek_type() != TokenType::Word {
            break;
        }
    }

    Ok(Box::new(nodes::PropertyList {
        properties_: properties
    }))
}

/*
Property
    PropertyDeclaration PropertyValue endOfLine
    PropertyDeclaration endOfLine PropertyBlock_opt
 */
fn parse_property<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::Property<'a>> {
    let declaration = parse_property_declaration(lexer)?;

    if lexer.peek_type() == TokenType::EndOfLine {
        let end_of_line = lexer.advance()?;

        if lexer.peek_type() != TokenType::Indent {
            // no optional block
            return Ok(Box::new(nodes::Property {
                declaration_: declaration,
                value_: None,
                end_of_line_: end_of_line,
                block_: None,
            }));
        }

        let block = parse_property_block(lexer)?;

        return Ok(Box::new(nodes::Property {
            declaration_: declaration,
            value_: None,
            end_of_line_: end_of_line,
            block_: Some(block),
        }));
    }

    let value = parse_property_value(lexer)?;
    let end_of_line = require_token(lexer, TokenType::EndOfLine)?;

    Ok(Box::new(nodes::Property {
        declaration_: declaration,
        value_: Some(value),
        end_of_line_: end_of_line,
        block_: None,
    }))
}

/*
PropertyDeclaration
    ## property type, followed by an optional name
    word word
    word quotedText
    word
 */
fn parse_property_declaration<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::PropertyDeclaration<'a>> {
    let prop_type = require_token(lexer, TokenType::Word)?;
    let peek = lexer.peek_type();
    if peek == TokenType::Word || peek == TokenType::QuotedText {
        let name = lexer.advance()?;
        return Ok(Box::new(nodes::PropertyDeclaration {
            type_: prop_type,
            name_: Some(name),
        }));
    }

    Ok(Box::new(nodes::PropertyDeclaration {
        type_: prop_type,
        name_: None,
    }))
}

/*
PropertyValue
    : quotedText
    : lineText
    > blockText
 */
fn parse_property_value<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::PropertyValue<'a>> {
    match lexer.peek_type() {
        TokenType::Colon => {
            let colon = lexer.advance()?;
            match lexer.peek_type() {
                TokenType::LineText |
                TokenType::QuotedText => {
                    let text = lexer.advance()?;
                    Ok(Box::new(nodes::PropertyValue {
                        specifier_: colon,
                        text_: text,
                    }))
                },
                _ => Err(unexpected_token(lexer, &[])),
            }
        },
        TokenType::GreaterThan => {
            let greater_than = lexer.advance()?;
            let block_text = require_token(lexer, TokenType::BlockText)?;
            Ok(Box::new(nodes::PropertyValue {
                specifier_: greater_than,
                text_: block_text,
            }))
        },
        _ => Err(unexpected_token(lexer, &[TokenType::Colon, TokenType::GreaterThan])),
    }
}

/*
PropertyBlock
    indent PropertyList outdent
 */
fn parse_property_block<'a>(lexer: &mut Lexer<'a>) -> ParserResult<nodes::PropertyBlock<'a>> {
    let indent = require_token(lexer, TokenType::Indent)?;
    let prop_list = parse_property_list(lexer)?;
    let outdent = require_token(lexer, TokenType::Outdent)?;
    Ok(Box::new(nodes::PropertyBlock {
        indent_: indent,
        property_list_: prop_list,
        outdent_: outdent,
    }))
}

fn require_token<'a>(lexer: &mut Lexer<'a>, token_type: TokenType) -> ParserResult<Token<'a>> {
    if lexer.peek_type() == token_type {
        return lexer.advance();
    }

    Err(unexpected_token(lexer, &[token_type]))
}

fn unexpected_token<'a>(lexer: &mut Lexer<'a>, expected_types: &[TokenType]) -> ParsingError {
    let token = match lexer.advance() {
        Ok(token) => token,
        Err(err) => return err,
    };

    let expected_str = format!("{:?}", expected_types);
    let message = format!("Unexpected Token `{:?}`\n    Expected: {}",
                          token.token_type,
                          &expected_str[1..expected_str.len()-1] // remove brackets
    );

    ParsingError::new(lexer.content, token.text_start, &message)
}
