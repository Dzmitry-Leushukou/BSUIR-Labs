use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq)]
pub enum ConstantValue {
    Int(i64),
    Hex(u64),
    Float(String),
    String(String),
    Char(String),
}

impl ConstantValue {
    pub fn type_name(&self) -> &str {
        match self {
            ConstantValue::Int(_) => "Int",
            ConstantValue::Hex(_) => "Hex",
            ConstantValue::Float(_) => "Double",
            ConstantValue::String(_) => "String",
            ConstantValue::Char(_) => "Char",
        }
    }

    pub fn value_string(&self) -> String {
        match self {
            ConstantValue::Int(n) => n.to_string(),
            ConstantValue::Hex(n) => format!("0x{:X}", n),
            ConstantValue::Float(s) => s.clone(),
            ConstantValue::String(s) => format!("\"{}\"", escape_string(s)),
            ConstantValue::Char(s) => format!("'{}'", escape_string(s)),
        }
    }

    pub fn value_string_raw(&self) -> String {

        match self {
            ConstantValue::Int(n) => n.to_string(),
            ConstantValue::Hex(n) => format!("0x{:X}", n),
            ConstantValue::Float(s) => s.clone(),
            ConstantValue::String(s) => format!("\"{}\"", escape_string(s)),
            ConstantValue::Char(s) => format!("'{}'", escape_string(s)),
        }
    }

    pub fn canonical_key(&self) -> String {
        match self {
            ConstantValue::Int(n) => format!("INT:{}", n),
            ConstantValue::Hex(n) => format!("HEX:{}", n),
            ConstantValue::Float(s) => format!("FLOAT:{}", s),
            ConstantValue::String(s) => format!("STRING:{}", s),
            ConstantValue::Char(s) => format!("CHAR:{}", s),
        }
    }
}

fn escape_string(s: &str) -> String {
    s.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
        .replace("\t", "\\t")
        .replace("\r", "\\r")
}

pub struct IdentifierTable {
    identifiers: Vec<String>,
    indices: HashMap<String, usize>,
}

impl IdentifierTable {
    pub fn new() -> Self {
        IdentifierTable {
            identifiers: Vec::new(),
            indices: HashMap::new(),
        }
    }

    pub fn get_or_insert(&mut self, name: &str) -> usize {
        if let Some(&idx) = self.indices.get(name) {
            idx
        } else {
            let idx = self.identifiers.len() + 1;
            self.identifiers.push(name.to_string());
            self.indices.insert(name.to_string(), idx);
            idx
        }
    }

    pub fn get(&self, name: &str) -> Option<usize> {
        self.indices.get(name).copied()
    }

    pub fn list(&self) -> Vec<(usize, &str)> {
        self.identifiers
            .iter()
            .enumerate()
            .map(|(i, s)| (i + 1, s.as_str()))
            .collect()
    }
}

pub struct ConstantTable {
    constants: Vec<ConstantValue>,
    indices: HashMap<String, usize>,
}

impl ConstantTable {
    pub fn new() -> Self {
        ConstantTable {
            constants: Vec::new(),
            indices: HashMap::new(),
        }
    }

    pub fn get_or_insert(&mut self, value: ConstantValue) -> usize {
        let key = value.canonical_key();
        if let Some(&idx) = self.indices.get(&key) {
            idx
        } else {
            let idx = self.constants.len() + 1;
            self.constants.push(value);
            self.indices.insert(key, idx);
            idx
        }
    }

    pub fn get(&self, value: &ConstantValue) -> Option<usize> {
        let key = value.canonical_key();
        self.indices.get(&key).copied()
    }

    pub fn list(&self) -> Vec<(usize, &ConstantValue)> {
        self.constants
            .iter()
            .enumerate()
            .map(|(i, v)| (i + 1, v))
            .collect()
    }
}

#[derive(Debug, Clone)]
pub enum TableEntry {
    Identifier(String),
    Constant(ConstantValue),
}

impl TableEntry {
    pub fn display_value(&self) -> String {
        match self {
            TableEntry::Identifier(name) => name.clone(),
            TableEntry::Constant(val) => val.value_string_raw(),
        }
    }
}

pub struct UnifiedTable {
    entries: Vec<TableEntry>,
    indices: HashMap<String, usize>,
}

impl UnifiedTable {
    pub fn new() -> Self {
        UnifiedTable {
            entries: Vec::new(),
            indices: HashMap::new(),
        }
    }

    pub fn add_identifier(&mut self, name: &str) -> usize {
        let key = format!("IDENT:{}", name);
        if let Some(&idx) = self.indices.get(&key) {
            idx
        } else {
            let idx = self.entries.len() + 1;
            self.entries.push(TableEntry::Identifier(name.to_string()));
            self.indices.insert(key, idx);
            idx
        }
    }

    pub fn add_constant(&mut self, value: ConstantValue) -> usize {
        let key = format!("CONST:{}", value.canonical_key());
        if let Some(&idx) = self.indices.get(&key) {
            idx
        } else {
            let idx = self.entries.len() + 1;
            self.entries.push(TableEntry::Constant(value));
            self.indices.insert(key, idx);
            idx
        }
    }

    pub fn get_identifier_index(&self, name: &str) -> Option<usize> {
        let key = format!("IDENT:{}", name);
        self.indices.get(&key).copied()
    }

    pub fn get_constant_index(&self, value: &ConstantValue) -> Option<usize> {
        let key = format!("CONST:{}", value.canonical_key());
        self.indices.get(&key).copied()
    }

    pub fn list(&self) -> Vec<(usize, &TableEntry)> {
        self.entries
            .iter()
            .enumerate()
            .map(|(i, e)| (i + 1, e))
            .collect()
    }
}