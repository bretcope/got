use super::*;
use std::fmt;

#[derive(Debug)]
pub enum NodeType<'a> {
    File(&'a File<'a>),
    PropertyList(&'a PropertyList<'a>),
    Property(&'a Property<'a>),
    PropertyDeclaration(&'a PropertyDeclaration<'a>),
    PropertyValue(&'a PropertyValue<'a>),
    PropertyBlock(&'a PropertyBlock<'a>),
}

pub trait Node<'a>: fmt::Debug {
    fn node_type(&self) -> NodeType;

    fn as_node(&self) -> &Node<'a>;

    fn as_syntax_element(&'a self) -> SyntaxElement<'a> {
        SyntaxElement::Node(self.as_node())
    }

    fn syntax_elements(&'a self, list: &mut Vec<SyntaxElement<'a>>);
}

// =================================================================================================
// File
// =================================================================================================

#[derive(Debug)]
pub struct File<'a> {
    property_list_: Box<PropertyList<'a>>,
    end_of_input_: Box<Token<'a>>
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
}

// =================================================================================================
// PropertyList
// =================================================================================================

#[derive(Debug)]
pub struct PropertyList<'a> {
    properties_: Vec<Box<Property<'a>>>,
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
}

// =================================================================================================
// Property
// =================================================================================================

#[derive(Debug)]
pub struct Property<'a> {
    declaration_: Box<PropertyDeclaration<'a>>,
    value_: Option<Box<PropertyValue<'a>>>,
    end_of_line_: Box<Token<'a>>,
    block_: Option<Box<PropertyBlock<'a>>>,
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
}

// =================================================================================================
// PropertyDeclaration
// =================================================================================================

#[derive(Debug)]
pub struct PropertyDeclaration<'a> {
    type_: Box<Token<'a>>,
    name_: Option<Box<Token<'a>>>,
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
}

// =================================================================================================
// PropertyValue
// =================================================================================================

#[derive(Debug)]
pub struct PropertyValue<'a> {
    specifier_: Box<Token<'a>>,
    text_: Box<Token<'a>>,
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
}

// =================================================================================================
// PropertyBlock
// =================================================================================================

#[derive(Debug)]
pub struct PropertyBlock<'a> {
    indent_: Box<Token<'a>>,
    property_list_: Box<PropertyList<'a>>,
    outdent_: Box<Token<'a>>,
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
}
