use std::collections::HashMap;

/// Entry in the constant table
#[derive(Debug, Clone, PartialEq)]
pub enum ConstantValue {
    Int(i64),
    Hex(u64),
    Float(String), // Store as string to preserve precision
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

/// Identifier table
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

/// Constant table
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
