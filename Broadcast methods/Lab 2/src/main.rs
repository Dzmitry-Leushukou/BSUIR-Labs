mod token;
mod symbol_table;
mod lexer;

use std::env;
use std::fs;
use std::process;

use lexer::Lexer;
use token::TokenType;
use symbol_table::{UnifiedTable, ConstantValue};

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Usage: {} <input_file>", args[0]);
        process::exit(1);
    }

    let filename = &args[1];
    
    let content = match fs::read_to_string(filename) {
        Ok(c) => c,
        Err(e) => {
            eprintln!("Error reading file '{}': {}", filename, e);
            process::exit(1);
        }
    };

    let mut lexer = Lexer::new(&content);
    let (tokens, errors) = lexer.tokenize();

    if !errors.is_empty() {
        for error in &errors {
            println!(
                "Лексическая ошибка в строке {}, колонка {}: {}",
                error.line, error.column, error.message
            );
        }
        process::exit(1);
    }

    let mut unified_table = UnifiedTable::new();
    
    for token in &tokens {
        match &token.token_type {
            TokenType::Identifier(name) => {
                unified_table.add_identifier(name);
            }
            TokenType::IntLiteral(v) => {
                if let Ok(val) = v.parse::<i64>() {
                    unified_table.add_constant(ConstantValue::Int(val));
                }
            }
            TokenType::HexLiteral(v) => {
                if let Ok(val) = u64::from_str_radix(&v[2..], 16) {
                    unified_table.add_constant(ConstantValue::Hex(val));
                }
            }
            TokenType::FloatLiteral(v) => {
                unified_table.add_constant(ConstantValue::Float(v.clone()));
            }
            TokenType::StringLiteral(v) => {
                unified_table.add_constant(ConstantValue::String(v.clone()));
            }
            TokenType::CharLiteral(v) => {
                unified_table.add_constant(ConstantValue::Char(v.clone()));
            }
            _ => {}
        }
    }

    println!("CONSTANTS AND IDENTS");
    println!();
    println!("ID    | Value");
    println!("---   | ---");
    
    // Collect all tokens with their IDs in order of appearance  
    let mut token_entries: Vec<(usize, String)> = Vec::new();
    let mut current_token_id = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        let token_str = match &token.token_type {
            TokenType::Keyword(kw) => kw.clone(),
            TokenType::Identifier(name) => name.clone(),
            TokenType::IntLiteral(v) => v.clone(),
            TokenType::HexLiteral(v) => v.clone(),
            TokenType::FloatLiteral(v) => v.clone(),
            TokenType::StringLiteral(v) => format!("\"{}\"", v),
            TokenType::CharLiteral(v) => format!("'{}'", v),
            TokenType::Plus => "+".to_string(),
            TokenType::Minus => "-".to_string(),
            TokenType::Star => "*".to_string(),
            TokenType::Slash => "/".to_string(),
            TokenType::Percent => "%".to_string(),
            TokenType::Caret => "^".to_string(),
            TokenType::Equal => "=".to_string(),
            TokenType::EqualEqual => "==".to_string(),
            TokenType::NotEqual => "/=".to_string(),
            TokenType::Less => "<".to_string(),
            TokenType::Greater => ">".to_string(),
            TokenType::LessEqual => "<=".to_string(),
            TokenType::GreaterEqual => ">=".to_string(),
            TokenType::AmpAmp => "&&".to_string(),
            TokenType::PipePipe => "||".to_string(),
            TokenType::Bang => "!".to_string(),
            TokenType::Colon => ":".to_string(),
            TokenType::ColonColon => "::".to_string(),
            TokenType::Arrow => "->".to_string(),
            TokenType::FatArrow => "=>".to_string(),
            TokenType::Backslash => "\\".to_string(),
            TokenType::Pipe => "|".to_string(),
            TokenType::At => "@".to_string(),
            TokenType::Question => "?".to_string(),
            TokenType::DotDot => "..".to_string(),
            TokenType::DotDotDot => "...".to_string(),
            TokenType::DollarSign => "$".to_string(),
            TokenType::Tilde => "~".to_string(),
            TokenType::ColonGreater => ":>".to_string(),
            TokenType::LessColon => "<:".to_string(),
            TokenType::LeftParen => "(".to_string(),
            TokenType::RightParen => ")".to_string(),
            TokenType::LeftBracket => "[".to_string(),
            TokenType::RightBracket => "]".to_string(),
            TokenType::LeftBrace => "{".to_string(),
            TokenType::RightBrace => "}".to_string(),
            TokenType::Comma => ",".to_string(),
            TokenType::Semicolon => ";".to_string(),
            TokenType::Dot => ".".to_string(),
            TokenType::Backtick => "`".to_string(),
            TokenType::Eof => break,
        };
        
        token_entries.push((current_token_id, token_str));
        current_token_id += 1;
    }
    
    // Output all tokens
    for (id, token_str) in token_entries {
        println!("{}    | {}", id, token_str);
    }
    
    println!();
    println!();
    
    println!("KEYWORDS TABLE");
    println!();
    println!("ID    | Keyword");
    println!("---   | ---");
    
    // Build keyword map (first occurrence only)
    let mut kw_token_ids: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    let mut current_token_id = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        match &token.token_type {
            TokenType::Keyword(kw) => {
                if !kw_token_ids.contains_key(kw) {
                    kw_token_ids.insert(kw.clone(), current_token_id);
                }
            }
            _ => {}
        }
        
        current_token_id += 1;
    }
    
    // Output keywords with their first occurrence ID
    let keywords = vec!["module", "import", "as", "data", "where", "let", "in", "if", "then", "else",
        "case", "of", "do", "record", "constructor", "interface", "implementation", "forall",
        "with", "mutual", "total", "partial", "covering", "impossible", "export", "private", "public",
        "parameters", "using", "namespace", "rewrite", "replace",
        "auto", "infixl", "infixr", "infix", "prefix",
        "Int", "Integer", "Nat", "Double", "String", "Char", "Bool", "Type", "IO",
        "List", "Vect", "Maybe", "Either", "Pair", "Void",
        "Functor", "Applicative", "Monad", "Alternative",
        "Eq", "Ord", "Show", "Monoid", "Semigroup",
        "Foldable", "Traversable", "HasIO"];
    
    for kw in keywords {
        if let Some(id) = kw_token_ids.get(kw) {
            println!("{}    | {}", id, kw);
        }
    }
    
    println!();
    println!();
    
    println!("DELIMITERS TABLE");
    println!();
    println!("ID    | Symbol | Name");
    println!("---   | ---    | ---");
    
    // Build delimiter map (first occurrence only)
    let mut delim_token_ids: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    let mut current_token_id = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        match &token.token_type {
            TokenType::LeftParen => { if !delim_token_ids.contains_key("(") { delim_token_ids.insert("(".to_string(), current_token_id); } }
            TokenType::RightParen => { if !delim_token_ids.contains_key(")") { delim_token_ids.insert(")".to_string(), current_token_id); } }
            TokenType::LeftBracket => { if !delim_token_ids.contains_key("[") { delim_token_ids.insert("[".to_string(), current_token_id); } }
            TokenType::RightBracket => { if !delim_token_ids.contains_key("]") { delim_token_ids.insert("]".to_string(), current_token_id); } }
            TokenType::LeftBrace => { if !delim_token_ids.contains_key("{") { delim_token_ids.insert("{".to_string(), current_token_id); } }
            TokenType::RightBrace => { if !delim_token_ids.contains_key("}") { delim_token_ids.insert("}".to_string(), current_token_id); } }
            TokenType::Comma => { if !delim_token_ids.contains_key(",") { delim_token_ids.insert(",".to_string(), current_token_id); } }
            TokenType::Semicolon => { if !delim_token_ids.contains_key(";") { delim_token_ids.insert(";".to_string(), current_token_id); } }
            TokenType::Dot => { if !delim_token_ids.contains_key(".") { delim_token_ids.insert(".".to_string(), current_token_id); } }
            TokenType::Backtick => { if !delim_token_ids.contains_key("`") { delim_token_ids.insert("`".to_string(), current_token_id); } }
            _ => {}
        }
        
        current_token_id += 1;
    }
    
    let delim_pairs = vec![("(", "LPAREN"), (")", "RPAREN"), ("[", "LBRACKET"), ("]", "RBRACKET"),
        ("{", "LBRACE"), ("}", "RBRACE"), (",", "COMMA"), (";", "SEMICOLON"), (".", "DOT"), ("`", "BACKTICK")];
    for (sym, name) in delim_pairs {
        if let Some(id) = delim_token_ids.get(sym) {
            println!("{}    | {}      | {}", id, sym, name);
        }
    }
    
    println!();
    println!();
    
    println!("OPERATORS TABLE");
    println!();
    
    // Build operator map (first occurrence only)
    let mut op_token_ids: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    let mut current_token_id = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        match &token.token_type {
            TokenType::Plus => { if !op_token_ids.contains_key("+") { op_token_ids.insert("+".to_string(), current_token_id); } }
            TokenType::Minus => { if !op_token_ids.contains_key("-") { op_token_ids.insert("-".to_string(), current_token_id); } }
            TokenType::Star => { if !op_token_ids.contains_key("*") { op_token_ids.insert("*".to_string(), current_token_id); } }
            TokenType::Slash => { if !op_token_ids.contains_key("/") { op_token_ids.insert("/".to_string(), current_token_id); } }
            TokenType::Percent => { if !op_token_ids.contains_key("%") { op_token_ids.insert("%".to_string(), current_token_id); } }
            TokenType::Caret => { if !op_token_ids.contains_key("^") { op_token_ids.insert("^".to_string(), current_token_id); } }
            TokenType::EqualEqual => { if !op_token_ids.contains_key("==") { op_token_ids.insert("==".to_string(), current_token_id); } }
            TokenType::NotEqual => { if !op_token_ids.contains_key("/=") { op_token_ids.insert("/=".to_string(), current_token_id); } }
            TokenType::Less => { if !op_token_ids.contains_key("<") { op_token_ids.insert("<".to_string(), current_token_id); } }
            TokenType::Greater => { if !op_token_ids.contains_key(">") { op_token_ids.insert(">".to_string(), current_token_id); } }
            TokenType::LessEqual => { if !op_token_ids.contains_key("<=") { op_token_ids.insert("<=".to_string(), current_token_id); } }
            TokenType::GreaterEqual => { if !op_token_ids.contains_key(">=") { op_token_ids.insert(">=".to_string(), current_token_id); } }
            TokenType::AmpAmp => { if !op_token_ids.contains_key("&&") { op_token_ids.insert("&&".to_string(), current_token_id); } }
            TokenType::PipePipe => { if !op_token_ids.contains_key("||") { op_token_ids.insert("||".to_string(), current_token_id); } }
            TokenType::Bang => { if !op_token_ids.contains_key("!") { op_token_ids.insert("!".to_string(), current_token_id); } }
            TokenType::Pipe => { if !op_token_ids.contains_key("|") { op_token_ids.insert("|".to_string(), current_token_id); } }
            TokenType::At => { if !op_token_ids.contains_key("@") { op_token_ids.insert("@".to_string(), current_token_id); } }
            TokenType::Colon => { if !op_token_ids.contains_key(":") { op_token_ids.insert(":".to_string(), current_token_id); } }
            TokenType::ColonColon => { if !op_token_ids.contains_key("::") { op_token_ids.insert("::".to_string(), current_token_id); } }
            TokenType::Arrow => { if !op_token_ids.contains_key("->") { op_token_ids.insert("->".to_string(), current_token_id); } }
            TokenType::FatArrow => { if !op_token_ids.contains_key("=>") { op_token_ids.insert("=>".to_string(), current_token_id); } }
            TokenType::Backslash => { if !op_token_ids.contains_key("\\") { op_token_ids.insert("\\".to_string(), current_token_id); } }
            TokenType::Question => { if !op_token_ids.contains_key("?") { op_token_ids.insert("?".to_string(), current_token_id); } }
            TokenType::DotDot => { if !op_token_ids.contains_key("..") { op_token_ids.insert("..".to_string(), current_token_id); } }
            TokenType::DotDotDot => { if !op_token_ids.contains_key("...") { op_token_ids.insert("...".to_string(), current_token_id); } }
            TokenType::DollarSign => { if !op_token_ids.contains_key("$") { op_token_ids.insert("$".to_string(), current_token_id); } }
            TokenType::Tilde => { if !op_token_ids.contains_key("~") { op_token_ids.insert("~".to_string(), current_token_id); } }
            TokenType::ColonGreater => { if !op_token_ids.contains_key(":>") { op_token_ids.insert(":>".to_string(), current_token_id); } }
            TokenType::LessColon => { if !op_token_ids.contains_key("<:") { op_token_ids.insert("<:".to_string(), current_token_id); } }
            TokenType::Equal => { if !op_token_ids.contains_key("=") { op_token_ids.insert("=".to_string(), current_token_id); } }
            _ => {}
        }
        
        current_token_id += 1;
    }
    
    println!("Arithmetic Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["+", "-", "*", "/", "%", "^"] {
        if let Some(id) = op_token_ids.get(sym) {
            println!("{}    | {}      | Arithmetic", id, sym);
        }
    }
    println!();
    
    println!("Comparison Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["==", "/=", "<", ">", "<=", ">="] {
        if let Some(id) = op_token_ids.get(sym) {
            println!("{}    | {}      | Comparison", id, sym);
        }
    }
    println!();
    
    println!("Logical Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["&&", "||", "!"] {
        if let Some(id) = op_token_ids.get(sym) {
            println!("{}    | {}      | Logical", id, sym);
        }
    }
    println!();
    
    println!("Bitwise Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["|", "&"] {
        if let Some(id) = op_token_ids.get(sym) {
            println!("{}    | {}      | Bitwise", id, sym);
        }
    }
    println!();
    
    println!("Special Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec![":", "::", "->", "=>", "\\", "@", "?", "..", "...", "$", "~", ":>", "<:", "="] {
        if let Some(id) = op_token_ids.get(sym) {
            println!("{}    | {}      | Special", id, sym);
        }
    }
    
    
    println!();
    
    // Token output with sequential IDs
    let mut transformed = String::new();
    let mut current_line = 1;
    let mut token_id = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        // Output newlines to preserve source file structure (including empty lines)
        while token.line > current_line {
            transformed.push('\n');
            current_line += 1;
        }
        
        // Generate token ID representation (sequential numbering)
        let token_str = format!("<ID{}>", token_id);
        token_id += 1;
        
        if !transformed.is_empty() && !transformed.ends_with('\n') {
            transformed.push(' ');
        }
        transformed.push_str(&token_str);
    }
    
    println!("{}", transformed);
}
