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
    
    for (idx, entry) in unified_table.list() {
        println!("{}    | {}", idx, entry.display_value());
    }
    
    println!();
    
    let mut transformed = String::new();
    
    for token in &tokens {
        if token.token_type == TokenType::Eof {
            break;
        }
        
        let token_str = match &token.token_type {
            TokenType::Keyword(kw) => kw.clone(),
            TokenType::Identifier(name) => {
                let idx = unified_table.get_identifier_index(name).unwrap();
                format!("<ID{}>", idx)
            }
            TokenType::IntLiteral(v) => {
                if let Ok(val) = v.parse::<i64>() {
                    let idx = unified_table.get_constant_index(&ConstantValue::Int(val)).unwrap();
                    format!("<ID{}>", idx)
                } else {
                    v.clone()
                }
            }
            TokenType::HexLiteral(v) => {
                if let Ok(val) = u64::from_str_radix(&v[2..], 16) {
                    let idx = unified_table.get_constant_index(&ConstantValue::Hex(val)).unwrap();
                    format!("<ID{}>", idx)
                } else {
                    v.clone()
                }
            }
            TokenType::FloatLiteral(v) => {
                let idx = unified_table.get_constant_index(&ConstantValue::Float(v.clone())).unwrap();
                format!("<ID{}>", idx)
            }
            TokenType::StringLiteral(v) => {
                let idx = unified_table.get_constant_index(&ConstantValue::String(v.clone())).unwrap();
                format!("<ID{}>", idx)
            }
            TokenType::CharLiteral(v) => {
                let idx = unified_table.get_constant_index(&ConstantValue::Char(v.clone())).unwrap();
                format!("<ID{}>", idx)
            }
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
            TokenType::Eof => "".to_string(),
        };
        
        if !token_str.is_empty() {
            if !transformed.is_empty() {
                transformed.push(' ');
            }
            transformed.push_str(&token_str);
        }
    }
    
    println!("{}", transformed);
}