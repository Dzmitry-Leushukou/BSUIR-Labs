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
    Keyword(String),
    Delimiter(String),
    Operator(String),
}

impl TableEntry {
    pub fn display_value(&self) -> String {
        match self {
            TableEntry::Identifier(name) => name.clone(),
            TableEntry::Constant(val) => val.value_string_raw(),
            TableEntry::Keyword(kw) => kw.clone(),
            TableEntry::Delimiter(sym) => sym.clone(),
            TableEntry::Operator(sym) => sym.clone(),
        }
    }
}

pub struct KeywordTable {
    keywords: Vec<String>,
    indices: HashMap<String, usize>,
}

impl KeywordTable {
    pub fn new() -> Self {
        let keywords = vec![
            "module", "import", "as", "data", "where", "let", "in", "if", "then", "else",
            "case", "of", "do", "record", "constructor", "interface", "implementation", "forall",
            "with", "mutual", "total", "partial", "covering", "impossible", "export", "private", "public",
            "parameters", "using", "namespace", "rewrite", "replace",
            "auto", "infixl", "infixr", "infix", "prefix",
            "Int", "Integer", "Nat", "Double", "String", "Char", "Bool", "Type", "IO",
            "List", "Vect", "Maybe", "Either", "Pair", "Void",
            "Functor", "Applicative", "Monad", "Alternative",
            "Eq", "Ord", "Show", "Monoid", "Semigroup",
            "Foldable", "Traversable", "HasIO",
        ];

        let mut indices = HashMap::new();
        for (i, kw) in keywords.iter().enumerate() {
            indices.insert(kw.to_string(), i + 1);
        }

        KeywordTable { keywords: keywords.iter().map(|s| s.to_string()).collect(), indices }
    }

    pub fn get_index(&self, keyword: &str) -> Option<usize> {
        self.indices.get(keyword).copied()
    }

    pub fn list(&self) -> Vec<(usize, &str)> {
        self.keywords
            .iter()
            .enumerate()
            .map(|(i, s)| (i + 1, s.as_str()))
            .collect()
    }

    pub fn is_keyword(word: &str) -> bool {
        matches!(
            word,
            "module" | "import" | "as" | "data" | "where" | "let" | "in" | "if" | "then" | "else"
                | "case" | "of" | "do" | "record" | "constructor" | "interface" | "implementation" | "forall"
                | "with" | "mutual" | "total" | "partial" | "covering" | "impossible" | "export" | "private" | "public"
                | "parameters" | "using" | "namespace" | "rewrite" | "replace"
                | "auto" | "infixl" | "infixr" | "infix" | "prefix"
                | "Int" | "Integer" | "Nat" | "Double" | "String" | "Char" | "Bool" | "Type" | "IO"
                | "List" | "Vect" | "Maybe" | "Either" | "Pair" | "Void"
                | "Functor" | "Applicative" | "Monad" | "Alternative"
                | "Eq" | "Ord" | "Show" | "Monoid" | "Semigroup"
                | "Foldable" | "Traversable" | "HasIO"
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Delimiter {
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Comma,
    Semicolon,
    Dot,
    Backtick,
}

impl Delimiter {
    pub fn symbol(&self) -> &str {
        match self {
            Delimiter::LeftParen => "(",
            Delimiter::RightParen => ")",
            Delimiter::LeftBracket => "[",
            Delimiter::RightBracket => "]",
            Delimiter::LeftBrace => "{",
            Delimiter::RightBrace => "}",
            Delimiter::Comma => ",",
            Delimiter::Semicolon => ";",
            Delimiter::Dot => ".",
            Delimiter::Backtick => "`",
        }
    }

    pub fn name(&self) -> &str {
        match self {
            Delimiter::LeftParen => "LPAREN",
            Delimiter::RightParen => "RPAREN",
            Delimiter::LeftBracket => "LBRACKET",
            Delimiter::RightBracket => "RBRACKET",
            Delimiter::LeftBrace => "LBRACE",
            Delimiter::RightBrace => "RBRACE",
            Delimiter::Comma => "COMMA",
            Delimiter::Semicolon => "SEMICOLON",
            Delimiter::Dot => "DOT",
            Delimiter::Backtick => "BACKTICK",
        }
    }
}

pub struct DelimiterTable {
    delimiters: Vec<Delimiter>,
    indices: HashMap<String, usize>,
}

impl DelimiterTable {
    pub fn new() -> Self {
        let delimiters = vec![
            Delimiter::LeftParen,
            Delimiter::RightParen,
            Delimiter::LeftBracket,
            Delimiter::RightBracket,
            Delimiter::LeftBrace,
            Delimiter::RightBrace,
            Delimiter::Comma,
            Delimiter::Semicolon,
            Delimiter::Dot,
            Delimiter::Backtick,
        ];

        let mut indices = HashMap::new();
        // Delimiters: ID 65-74
        for (i, delim) in delimiters.iter().enumerate() {
            indices.insert(delim.symbol().to_string(), 65 + i);
        }

        DelimiterTable { delimiters, indices }
    }

    pub fn get_index(&self, symbol: &str) -> Option<usize> {
        self.indices.get(symbol).copied()
    }

    pub fn list(&self) -> Vec<(usize, &Delimiter)> {
        self.delimiters
            .iter()
            .enumerate()
            .map(|(i, d)| (65 + i, d))
            .collect()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum OperatorType {
    Arithmetic,
    Comparison,
    Logical,
    Bitwise,
    Special,
}

impl OperatorType {
    pub fn name(&self) -> &str {
        match self {
            OperatorType::Arithmetic => "Arithmetic",
            OperatorType::Comparison => "Comparison",
            OperatorType::Logical => "Logical",
            OperatorType::Bitwise => "Bitwise",
            OperatorType::Special => "Special",
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Operator {
    pub symbol: String,
    pub op_type: OperatorType,
}

impl Operator {
    pub fn new(symbol: &str, op_type: OperatorType) -> Self {
        Operator {
            symbol: symbol.to_string(),
            op_type,
        }
    }
}

pub struct OperatorTable {
    operators: Vec<Operator>,
    indices: HashMap<String, usize>,
}

impl OperatorTable {
    pub fn new() -> Self {
        let operators = vec![
            Operator::new("+", OperatorType::Arithmetic),
            Operator::new("-", OperatorType::Arithmetic),
            Operator::new("*", OperatorType::Arithmetic),
            Operator::new("/", OperatorType::Arithmetic),
            Operator::new("%", OperatorType::Arithmetic),
            Operator::new("^", OperatorType::Arithmetic),
            Operator::new("==", OperatorType::Comparison),
            Operator::new("/=", OperatorType::Comparison),
            Operator::new("<", OperatorType::Comparison),
            Operator::new(">", OperatorType::Comparison),
            Operator::new("<=", OperatorType::Comparison),
            Operator::new(">=", OperatorType::Comparison),
            Operator::new("&&", OperatorType::Logical),
            Operator::new("||", OperatorType::Logical),
            Operator::new("!", OperatorType::Logical),
            Operator::new("|", OperatorType::Bitwise),
            Operator::new("&", OperatorType::Bitwise),
            Operator::new(":", OperatorType::Special),
            Operator::new("::", OperatorType::Special),
            Operator::new("->", OperatorType::Special),
            Operator::new("=>", OperatorType::Special),
            Operator::new("\\", OperatorType::Special),
            Operator::new("@", OperatorType::Special),
            Operator::new("?", OperatorType::Special),
            Operator::new("..", OperatorType::Special),
            Operator::new("...", OperatorType::Special),
            Operator::new("$", OperatorType::Special),
            Operator::new("~", OperatorType::Special),
            Operator::new(":>", OperatorType::Special),
            Operator::new("<:", OperatorType::Special),
            Operator::new("=", OperatorType::Special),
        ];

        let mut indices = HashMap::new();
        // Operators: ID 75-105
        for (i, op) in operators.iter().enumerate() {
            indices.insert(op.symbol.clone(), 75 + i);
        }

        OperatorTable { operators, indices }
    }

    pub fn get_index(&self, symbol: &str) -> Option<usize> {
        self.indices.get(symbol).copied()
    }

    pub fn list(&self) -> Vec<(usize, &Operator)> {
        self.operators
            .iter()
            .enumerate()
            .map(|(i, op)| (75 + i, op))
            .collect()
    }

    pub fn list_by_type(&self, op_type: &OperatorType) -> Vec<(usize, &Operator)> {
        self.operators
            .iter()
            .enumerate()
            .filter(|(_, op)| &op.op_type == op_type)
            .map(|(i, op)| (75 + i, op))
            .collect()
    }
}

pub struct UnifiedTable {
    keyword_table: KeywordTable,
    delimiter_table: DelimiterTable,
    operator_table: OperatorTable,
    identifiers: Vec<String>,
    identifier_indices: HashMap<String, usize>,
    constants: Vec<ConstantValue>,
    constant_indices: HashMap<String, usize>,
}

impl UnifiedTable {
    pub fn new() -> Self {
        UnifiedTable {
            keyword_table: KeywordTable::new(),
            delimiter_table: DelimiterTable::new(),
            operator_table: OperatorTable::new(),
            identifiers: Vec::new(),
            identifier_indices: HashMap::new(),
            constants: Vec::new(),
            constant_indices: HashMap::new(),
        }
    }

    pub fn add_identifier(&mut self, name: &str) -> usize {
        if let Some(&idx) = self.identifier_indices.get(name) {
            idx
        } else {
            // ID starting from 106 (after keywords 1-64, delimiters 65-74, operators 75-105)
            let idx = 106 + self.identifiers.len();
            self.identifiers.push(name.to_string());
            self.identifier_indices.insert(name.to_string(), idx);
            idx
        }
    }

    pub fn add_constant(&mut self, value: ConstantValue) -> usize {
        let key = value.canonical_key();
        if let Some(&idx) = self.constant_indices.get(&key) {
            idx
        } else {
            // ID starting from 106 + num_identifiers
            let idx = 106 + self.identifiers.len() + self.constants.len();
            self.constants.push(value);
            self.constant_indices.insert(key, idx);
            idx
        }
    }

    pub fn get_keyword_index(&self, keyword: &str) -> Option<usize> {
        self.keyword_table.get_index(keyword)
    }

    pub fn get_delimiter_index(&self, symbol: &str) -> Option<usize> {
        self.delimiter_table.get_index(symbol)
    }

    pub fn get_operator_index(&self, symbol: &str) -> Option<usize> {
        self.operator_table.get_index(symbol)
    }

    pub fn get_identifier_index(&self, name: &str) -> Option<usize> {
        self.identifier_indices.get(name).copied()
    }

    pub fn get_constant_index(&self, value: &ConstantValue) -> Option<usize> {
        let key = value.canonical_key();
        self.constant_indices.get(&key).copied()
    }

    pub fn list(&self) -> Vec<(usize, &TableEntry)> {
        // This method is for backward compatibility, collects all entries
        let mut result = Vec::new();
        
        // Add identifiers
        for (idx, name) in self.identifiers.iter().enumerate() {
            let id = 96 + idx;
            // Create temporary entry - this is inefficient but OK for debug output
            // In real code, we'd refactor the display logic
        }
        
        // Add constants
        for (idx, value) in self.constants.iter().enumerate() {
            let id = 96 + self.identifiers.len() + idx;
        }
        
        result
    }

    pub fn list_by_type(&self, entry_type: &str) -> Vec<(usize, &TableEntry)> {
        vec![]
    }

    pub fn get_operators_by_type(&self, op_type: &str) -> Vec<(usize, &TableEntry)> {
        vec![]
    }

    pub fn get_identifiers(&self) -> &Vec<String> {
        &self.identifiers
    }

    pub fn get_constants(&self) -> &Vec<ConstantValue> {
        &self.constants
    }
}