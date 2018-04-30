use super::*;
use std::fmt;
use colors::*;

#[derive(Debug)]
pub enum NodeType<'a> {
    File(&'a File<'a>),
    PropertyList(&'a PropertyList<'a>),
    Property(&'a Property<'a>),
    PropertyDeclaration(&'a PropertyDeclaration<'a>),
    PropertyValue(&'a PropertyValue<'a>),
    PropertyBlock(&'a PropertyBlock<'a>),
}

pub trait Node<'a>: fmt::Debug + fmt::Display {
    fn node_type(&self) -> NodeType;

    fn as_node(&self) -> &Node<'a>;

    fn as_syntax_element(&'a self) -> SyntaxElement<'a> {
        SyntaxElement::Node(self.as_node())
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>);

    fn content_position(&'a self) -> (&'a FileContent, usize) {
        let mut list = Vec::new();
        let mut element = self.as_syntax_element();
        while let SyntaxElement::Node(node) = element {
            list.clear();
            node.syntax_elements(&mut list);
            element = list[0];
        }

        if let SyntaxElement::Token(token) = element {
            return (token.content, token.text_start);
        }

        panic!("Could not find token for node: {}", self)
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result;
}

fn format_indent(level: usize) -> String {
    format!("{}{}{}", GREY, "|   ".repeat(level), RESET)
}

fn format_node_type(indent_level: usize, type_name: &str) -> String {
    format!("{}{}{}{}", format_indent(indent_level), CYAN, type_name, RESET)
}

// =================================================================================================
// File
// =================================================================================================

pub struct File<'a> {
    pub(super) property_list_: Box<PropertyList<'a>>,
    pub(super) end_of_input_: Box<Token<'a>>
}

impl<'a> File<'a> {
    pub fn property_list(&self) -> &PropertyList<'a> {
        &self.property_list_
    }
}

impl<'a> Node<'a> for File<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::File(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        list.push(self.property_list_.as_syntax_element());
        list.push(SyntaxElement::Token(&self.end_of_input_));
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        writeln!(f, "{} {}", format_node_type(indent_level, "File:"), self.end_of_input_.content.filename())?;
        self.property_list_.print(f, indent_level + 1)
    }
}

impl<'a> fmt::Debug for File<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for File<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

// =================================================================================================
// PropertyList
// =================================================================================================

pub struct PropertyList<'a> {
    pub(super) properties_: Vec<Box<Property<'a>>>,
}

impl<'a> PropertyList<'a> {
    pub fn properties(&self) -> &Vec<Box<Property<'a>>> {
        &self.properties_
    }
}

impl<'a> Node<'a> for PropertyList<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyList(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        for prop in &self.properties_ {
            list.push(prop.as_syntax_element());
        }
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        let prop_count = self.properties_.len();
        writeln!(f, "{} (count: {})", format_node_type(indent_level, "PropertyList"), prop_count)?;
        for prop in &self.properties_ {
            prop.print(f, indent_level + 1)?;
        }

        Ok(())
    }
}

impl<'a> fmt::Debug for PropertyList<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for PropertyList<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

// =================================================================================================
// Property
// =================================================================================================

pub struct Property<'a> {
    pub(super) declaration_: Box<PropertyDeclaration<'a>>,
    pub(super) value_: Option<Box<PropertyValue<'a>>>,
    pub(super) end_of_line_: Box<Token<'a>>,
    pub(super) block_: Option<Box<PropertyBlock<'a>>>,
}

impl<'a> Property<'a> {
    pub fn declaration(&self) -> &PropertyDeclaration {
        &self.declaration_
    }

    pub fn property_type(&self) -> &str {
        self.declaration().property_type()
    }

    pub fn property_name(&self) -> Option<&str> {
        self.declaration().property_name()
    }

    pub fn has_value(&self) -> bool {
        match &self.value_ {
            &Some(_) => true,
            &None => false,
        }
    }

    pub fn value(&'a self) -> Option<&'a PropertyValue<'a>> {
        match &self.value_ {
            &Some(ref value) => Some(value),
            &None => None,
        }
    }

    pub fn has_block(&self) -> bool {
        match &self.block_ {
            &Some(_) => true,
            &None => false,
        }
    }

    pub fn block(&'a self) -> Option<&'a PropertyBlock<'a>> {
        match &self.block_ {
            &Some(ref block) => Some(block),
            &None => None,
        }
    }
}

impl<'a> Node<'a> for Property<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::Property(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        list.push(self.declaration_.as_syntax_element());
        if let &Some(ref value) = &self.value_ {
            list.push(value.as_syntax_element());
        }
        list.push(SyntaxElement::Token(&self.end_of_line_));
        if let &Some(ref block) = &self.block_ {
            list.push(block.as_syntax_element());
        }
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        writeln!(f, "{}", format_node_type(indent_level, "Property"))?;
        self.declaration_.print(f, indent_level + 1)?;

        if let Some(value) = self.value() {
            value.print(f, indent_level + 1)?;
        }

        if let Some(block) = self.block() {
            block.print(f, indent_level + 1)?;
        }

        Ok(())
    }
}

impl<'a> fmt::Debug for Property<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for Property<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

// =================================================================================================
// PropertyDeclaration
// =================================================================================================

pub struct PropertyDeclaration<'a> {
    pub(super) type_: Box<Token<'a>>,
    pub(super) name_: Option<Box<Token<'a>>>,
}

impl<'a> PropertyDeclaration<'a> {
    pub fn property_type(&self) -> &str {
        self.type_.value_or_panic("`type_` token in PropertyDeclaration does not have a value.")
    }

    pub fn property_name(&self) -> Option<&str> {
        match &self.name_ {
            &Some(ref token) => Some(token.value_or_panic("`name_` token in PropertyDeclaration does not have a value.")),
            &None => None,
        }
    }

    pub fn has_name(&self) -> bool {
        match &self.name_ {
            &Some(_) => true,
            &None => false,
        }
    }
}

impl<'a> Node<'a> for PropertyDeclaration<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyDeclaration(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        list.push(self.type_.as_syntax_element());
        if let &Some(ref name) = &self.name_ {
            list.push(name.as_syntax_element());
        }
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        let prop_type = self.property_type();
        let name = match self.property_name() {
            Some(name) => format!(" \"{}\"", name),
            None => String::new(),
        };

        writeln!(f, "{} {}{}", format_node_type(indent_level, "Declaration:"), prop_type, name)
    }
}

impl<'a> fmt::Debug for PropertyDeclaration<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for PropertyDeclaration<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

// =================================================================================================
// PropertyValue
// =================================================================================================

pub struct PropertyValue<'a> {
    pub(super) specifier_: Box<Token<'a>>,
    pub(super) text_: Box<Token<'a>>,
}

impl<'a> PropertyValue<'a> {
    pub fn value(&self) -> &str {
        self.text_.value_or_panic("`text_` token in PropertyValue did not have a value.")
    }
}

impl<'a> Node<'a> for PropertyValue<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyValue(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        list.push(self.specifier_.as_syntax_element());
        list.push(self.text_.as_syntax_element());
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        writeln!(f, "{} \"{}\"", format_node_type(indent_level, "Value:"), self.value())
    }
}

impl<'a> fmt::Debug for PropertyValue<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for PropertyValue<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

// =================================================================================================
// PropertyBlock
// =================================================================================================

pub struct PropertyBlock<'a> {
    pub(super) indent_: Box<Token<'a>>,
    pub(super) property_list_: Box<PropertyList<'a>>,
    pub(super) outdent_: Box<Token<'a>>,
}

impl<'a> PropertyBlock<'a> {
    pub fn property_list(&self) -> &PropertyList<'a> {
        &self.property_list_
    }
}

impl<'a> Node<'a> for PropertyBlock<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyBlock(&self)
    }

    fn as_node(&self) -> &Node<'a> {
        self as &Node
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>) {
        list.push(self.indent_.as_syntax_element());
        list.push(self.property_list_.as_syntax_element());
        list.push(self.outdent_.as_syntax_element());
    }

    fn print(&self, f: &mut fmt::Formatter, indent_level: usize) -> fmt::Result {
        writeln!(f, "{}", format_node_type(indent_level, "Block"))?;
        self.property_list_.print(f, indent_level + 1)
    }
}

impl<'a> fmt::Debug for PropertyBlock<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}

impl<'a> fmt::Display for PropertyBlock<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.print(f, 0)
    }
}
