use super::*;

#[derive(Debug)]
pub enum NodeType<'a> {
    File(&'a File<'a>),
    PropertyList(&'a PropertyList<'a>),
    Property(&'a Property<'a>),
    PropertyDeclaration(&'a PropertyDeclaration<'a>),
    PropertyValue(&'a PropertyValue<'a>),
    PropertyBlock(&'a PropertyBlock<'a>),
}

pub trait Node<'a> {
    fn node_type(&self) -> NodeType;
}

// =================================================================================================
// File
// =================================================================================================

#[derive(Debug)]
pub struct File<'a> {
    property_list_: Box<PropertyList<'a>>,
    end_of_input_: Box<Token<'a>>
}

impl<'a> Node<'a> for File<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::File(&self)
    }
}

// =================================================================================================
// PropertyList
// =================================================================================================

#[derive(Debug)]
pub struct PropertyList<'a> {
    // todo
    a: &'a str,
}

impl<'a> Node<'a> for PropertyList<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyList(&self)
    }
}

// =================================================================================================
// Property
// =================================================================================================

#[derive(Debug)]
pub struct Property<'a> {
    // todo
    a: &'a str,
}

impl<'a> Node<'a> for Property<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::Property(&self)
    }
}

// =================================================================================================
// PropertyDeclaration
// =================================================================================================

#[derive(Debug)]
pub struct PropertyDeclaration<'a> {
    // todo
    a: &'a str,
}

impl<'a> Node<'a> for PropertyDeclaration<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyDeclaration(&self)
    }
}

// =================================================================================================
// PropertyValue
// =================================================================================================

#[derive(Debug)]
pub struct PropertyValue<'a> {
    // todo
    a: &'a str,
}

impl<'a> Node<'a> for PropertyValue<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyValue(&self)
    }
}

// =================================================================================================
// PropertyBlock
// =================================================================================================

#[derive(Debug)]
pub struct PropertyBlock<'a> {
    // todo
    a: &'a str,
}

impl<'a> Node<'a> for PropertyBlock<'a> {
    fn node_type(&self) -> NodeType {
        NodeType::PropertyBlock(&self)
    }
}
