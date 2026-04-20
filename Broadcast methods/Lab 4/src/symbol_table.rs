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

pub struct IdentifierTable {
    identifiers: Vec<String>,
    indices: HashMap<String, usize>,
}

impl IdentifierTable {
    pub fn new() -> Self {
        Self {
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
}

pub struct ConstantTable {
    constants: Vec<ConstantValue>,
    indices: HashMap<String, usize>,
}

impl ConstantTable {
    pub fn new() -> Self {
        Self {
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
}

pub struct KeywordTable;

impl KeywordTable {
    pub fn is_keyword(word: &str) -> bool {
        matches!(
            word,
            "module"
                | "import"
                | "as"
                | "data"
                | "where"
                | "let"
                | "in"
                | "if"
                | "then"
                | "else"
                | "case"
                | "of"
                | "do"
                | "record"
                | "constructor"
                | "interface"
                | "implementation"
                | "forall"
                | "with"
                | "mutual"
                | "total"
                | "partial"
                | "covering"
                | "impossible"
                | "export"
                | "private"
                | "public"
                | "parameters"
                | "using"
                | "namespace"
                | "rewrite"
                | "replace"
                | "auto"
                | "infixl"
                | "infixr"
                | "infix"
                | "prefix"
                | "Int"
                | "Integer"
                | "Nat"
                | "Double"
                | "String"
                | "Char"
                | "Bool"
                | "Type"
                | "IO"
                | "List"
                | "Vect"
                | "Maybe"
                | "Either"
                | "Pair"
                | "Void"
                | "Functor"
                | "Applicative"
                | "Monad"
                | "Alternative"
                | "Eq"
                | "Ord"
                | "Show"
                | "Monoid"
                | "Semigroup"
                | "Foldable"
                | "Traversable"
                | "HasIO"
        )
    }
}
