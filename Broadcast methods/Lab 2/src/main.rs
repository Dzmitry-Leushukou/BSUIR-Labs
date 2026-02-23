mod token;
mod symbol_table;
mod lexer;

use std::env;
use std::fs;
use std::process;

use lexer::Lexer;
use token::TokenType;

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

    use std::collections::HashMap;
    
    // Global token ID assignment - one ID per unique token, in order of first appearance
    let mut global_token_ids: HashMap<String, usize> = HashMap::new();
    let mut next_id = 1;
    let mut token_list: Vec<(String, String)> = Vec::new(); // (token_key, token_type_string)
    
    // First pass: assign global IDs to all unique tokens
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        let (token_key, token_type_str) = match &token.token_type {
            TokenType::Keyword(kw) => (format!("KW:{}", kw), "keyword".to_string()),
            TokenType::Identifier(name) => (format!("ID:{}", name), "identifier".to_string()),
            TokenType::IntLiteral(v) => (format!("INT:{}", v), "constant".to_string()),
            TokenType::HexLiteral(v) => (format!("HEX:{}", v), "constant".to_string()),
            TokenType::FloatLiteral(v) => (format!("FLOAT:{}", v), "constant".to_string()),
            TokenType::StringLiteral(v) => (format!("STRING:{}", v), "constant".to_string()),
            TokenType::CharLiteral(v) => (format!("CHAR:{}", v), "constant".to_string()),
            TokenType::Plus => ("+".to_string(), "operator".to_string()),
            TokenType::Minus => ("-".to_string(), "operator".to_string()),
            TokenType::Star => ("*".to_string(), "operator".to_string()),
            TokenType::Slash => ("/".to_string(), "operator".to_string()),
            TokenType::Percent => ("%".to_string(), "operator".to_string()),
            TokenType::Caret => ("^".to_string(), "operator".to_string()),
            TokenType::Equal => ("=".to_string(), "operator".to_string()),
            TokenType::EqualEqual => ("==".to_string(), "operator".to_string()),
            TokenType::NotEqual => ("/=".to_string(), "operator".to_string()),
            TokenType::Less => ("<".to_string(), "operator".to_string()),
            TokenType::Greater => (">".to_string(), "operator".to_string()),
            TokenType::LessEqual => ("<=".to_string(), "operator".to_string()),
            TokenType::GreaterEqual => (">=".to_string(), "operator".to_string()),
            TokenType::AmpAmp => ("&&".to_string(), "operator".to_string()),
            TokenType::PipePipe => ("||".to_string(), "operator".to_string()),
            TokenType::Bang => ("!".to_string(), "operator".to_string()),
            TokenType::Colon => (":".to_string(), "operator".to_string()),
            TokenType::ColonColon => ("::".to_string(), "operator".to_string()),
            TokenType::Arrow => ("->".to_string(), "operator".to_string()),
            TokenType::FatArrow => ("=>".to_string(), "operator".to_string()),
            TokenType::Backslash => ("\\".to_string(), "operator".to_string()),
            TokenType::Pipe => ("|".to_string(), "operator".to_string()),
            TokenType::At => ("@".to_string(), "operator".to_string()),
            TokenType::Question => ("?".to_string(), "operator".to_string()),
            TokenType::DotDot => ("..".to_string(), "operator".to_string()),
            TokenType::DotDotDot => ("...".to_string(), "operator".to_string()),
            TokenType::DollarSign => ("$".to_string(), "operator".to_string()),
            TokenType::Tilde => ("~".to_string(), "operator".to_string()),
            TokenType::ColonGreater => (":>".to_string(), "operator".to_string()),
            TokenType::LessColon => ("<:".to_string(), "operator".to_string()),
            TokenType::LeftParen => ("(".to_string(), "delimiter".to_string()),
            TokenType::RightParen => (")".to_string(), "delimiter".to_string()),
            TokenType::LeftBracket => ("[".to_string(), "delimiter".to_string()),
            TokenType::RightBracket => ("]".to_string(), "delimiter".to_string()),
            TokenType::LeftBrace => ("{".to_string(), "delimiter".to_string()),
            TokenType::RightBrace => ("}".to_string(), "delimiter".to_string()),
            TokenType::Comma => (",".to_string(), "delimiter".to_string()),
            TokenType::Semicolon => (";".to_string(), "delimiter".to_string()),
            TokenType::Dot => (".".to_string(), "delimiter".to_string()),
            TokenType::Backtick => ("`".to_string(), "delimiter".to_string()),
            TokenType::Eof => return,
        };
        
        if !global_token_ids.contains_key(&token_key) {
            global_token_ids.insert(token_key.clone(), next_id);
            token_list.push((token_key, token_type_str));
            next_id += 1;
        }
    }
    
    // Print KEYWORDS TABLE
    println!("KEYWORDS TABLE");
    println!();
    println!("ID    | Keyword");
    println!("---   | ---");
    for (token_key, id) in global_token_ids.iter() {
        if let Some(kw) = token_key.strip_prefix("KW:") {
            println!("{}    | {}", id, kw);
        }
    }
    
    println!();
    println!();
    
    // Print IDENTIFIERS TABLE
    println!("IDENTIFIERS TABLE");
    println!();
    println!("ID    | Identifier");
    println!("---   | ---");
    for (token_key, id) in global_token_ids.iter() {
        if let Some(name) = token_key.strip_prefix("ID:") {
            println!("{}    | {}", id, name);
        }
    }
    
    println!();
    println!();
    
    // Print CONSTANTS TABLE
    println!("CONSTANTS TABLE");
    println!();
    println!("ID    | Value          | Type");
    println!("---   | ---            | ---");
    for (token_key, id) in global_token_ids.iter() {
        if let Some(v) = token_key.strip_prefix("INT:") {
            if let Ok(val) = v.parse::<i64>() {
                println!("{}    | {}            | Int", id, val);
            }
        } else if let Some(v) = token_key.strip_prefix("HEX:") {
            if let Ok(val) = u64::from_str_radix(&v[2..], 16) {
                println!("{}    | {}            | Hex", id, format!("0x{:X}", val));
            }
        } else if let Some(v) = token_key.strip_prefix("FLOAT:") {
            println!("{}    | {}            | Double", id, v);
        } else if let Some(v) = token_key.strip_prefix("STRING:") {
            println!("{}    | \"{}\"            | String", id, v);
        } else if let Some(v) = token_key.strip_prefix("CHAR:") {
            println!("{}    | '{}'            | Char", id, v);
        }
    }
    
    println!();
    println!();
    
    println!("DELIMITERS TABLE");
    println!();
    println!("ID    | Symbol | Name");
    println!("---   | ---    | ---");
    
    let delim_pairs = vec![("(", "LPAREN"), (")", "RPAREN"), ("[", "LBRACKET"), ("]", "RBRACKET"),
        ("{", "LBRACE"), ("}", "RBRACE"), (",", "COMMA"), (";", "SEMICOLON"), (".", "DOT"), ("`", "BACKTICK")];
    for (sym, name) in delim_pairs {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | {}", id, sym, name);
        }
    }
    
    println!();
    println!();
    
    println!("OPERATORS TABLE");
    println!();
    
    println!("Arithmetic Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["+", "-", "*", "/", "%", "^"] {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | Arithmetic", id, sym);
        }
    }
    println!();
    
    println!("Comparison Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["==", "/=", "<", ">", "<=", ">="] {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | Comparison", id, sym);
        }
    }
    println!();
    
    println!("Logical Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["&&", "||", "!"] {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | Logical", id, sym);
        }
    }
    println!();
    
    println!("Bitwise Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec!["|", "&"] {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | Bitwise", id, sym);
        }
    }
    println!();
    
    println!("Special Operators:");
    println!("ID    | Symbol | Type");
    println!("---   | ---    | ---");
    for sym in vec![":", "::", "->", "=>", "\\", "@", "?", "..", "...", "$", "~", ":>", "<:", "="] {
        if let Some(id) = global_token_ids.get(sym) {
            println!("{}    | {}      | Special", id, sym);
        }
    }
    
    
    println!();
    
    // Token output with correct global IDs
    let mut transformed = String::new();
    let mut current_line = 1;
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        // Output newlines to preserve source file structure (including empty lines)
        while token.line > current_line {
            transformed.push('\n');
            current_line += 1;
        }
        
        // Generate token ID representation with correct global ID mapping
        let token_key = match &token.token_type {
            TokenType::Keyword(kw) => Some(format!("KW:{}", kw)),
            TokenType::Identifier(name) => Some(format!("ID:{}", name)),
            TokenType::IntLiteral(v) => Some(format!("INT:{}", v)),
            TokenType::HexLiteral(v) => Some(format!("HEX:{}", v)),
            TokenType::FloatLiteral(v) => Some(format!("FLOAT:{}", v)),
            TokenType::StringLiteral(v) => Some(format!("STRING:{}", v)),
            TokenType::CharLiteral(v) => Some(format!("CHAR:{}", v)),
            TokenType::Plus => Some("+".to_string()),
            TokenType::Minus => Some("-".to_string()),
            TokenType::Star => Some("*".to_string()),
            TokenType::Slash => Some("/".to_string()),
            TokenType::Percent => Some("%".to_string()),
            TokenType::Caret => Some("^".to_string()),
            TokenType::Equal => Some("=".to_string()),
            TokenType::EqualEqual => Some("==".to_string()),
            TokenType::NotEqual => Some("/=".to_string()),
            TokenType::Less => Some("<".to_string()),
            TokenType::Greater => Some(">".to_string()),
            TokenType::LessEqual => Some("<=".to_string()),
            TokenType::GreaterEqual => Some(">=".to_string()),
            TokenType::AmpAmp => Some("&&".to_string()),
            TokenType::PipePipe => Some("||".to_string()),
            TokenType::Bang => Some("!".to_string()),
            TokenType::Colon => Some(":".to_string()),
            TokenType::ColonColon => Some("::".to_string()),
            TokenType::Arrow => Some("->".to_string()),
            TokenType::FatArrow => Some("=>".to_string()),
            TokenType::Backslash => Some("\\".to_string()),
            TokenType::Pipe => Some("|".to_string()),
            TokenType::At => Some("@".to_string()),
            TokenType::Question => Some("?".to_string()),
            TokenType::DotDot => Some("..".to_string()),
            TokenType::DotDotDot => Some("...".to_string()),
            TokenType::DollarSign => Some("$".to_string()),
            TokenType::Tilde => Some("~".to_string()),
            TokenType::ColonGreater => Some(":>".to_string()),
            TokenType::LessColon => Some("<:".to_string()),
            TokenType::LeftParen => Some("(".to_string()),
            TokenType::RightParen => Some(")".to_string()),
            TokenType::LeftBracket => Some("[".to_string()),
            TokenType::RightBracket => Some("]".to_string()),
            TokenType::LeftBrace => Some("{".to_string()),
            TokenType::RightBrace => Some("}".to_string()),
            TokenType::Comma => Some(",".to_string()),
            TokenType::Semicolon => Some(";".to_string()),
            TokenType::Dot => Some(".".to_string()),
            TokenType::Backtick => Some("`".to_string()),
            TokenType::Eof => None,
        };
        
        if let Some(key) = token_key {
            if let Some(id) = global_token_ids.get(&key) {
                let token_str = format!("<ID{}>", id);
                if !transformed.is_empty() && !transformed.ends_with('\n') {
                    transformed.push(' ');
                }
                transformed.push_str(&token_str);
            }
        }
    }
    
    println!("{}", transformed);
}
