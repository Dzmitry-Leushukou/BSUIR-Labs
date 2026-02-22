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

    // Output errors first
    if !errors.is_empty() {
        println!("═══════════════════════════════════════");
        println!("ЛЕКСИЧЕСКИЕ ОШИБКИ:");
        println!("═══════════════════════════════════════");
        for error in &errors {
            println!(
                "Лексическая ошибка в строке {}, колонка {}: {}",
                error.line, error.column, error.message
            );
        }
        println!();
    }

    // Output tokens
    println!("═══════════════════════════════════════");
    println!("ТОКЕНЫ:");
    println!("═══════════════════════════════════════");
    
    let mut token_count = 0;
    for token in &tokens {
        token_count += 1;
        
        let token_desc = format_token(token);
        let location = format!("[строка {}, колонка {}]", token.line, token.column);
        
        print!("Токен {}: {} {}", token_count, token_desc, location);
        
        if let Some(idx) = token.table_index {
            print!(" (ID={})", idx);
        }
        
        println!();
        
        if token.token_type == TokenType::Eof {
            break;
        }
    }

    println!();

    // Output identifier table
    let id_table = lexer.get_identifier_table();
    let id_list = id_table.list();
    
    if !id_list.is_empty() {
        println!("═══════════════════════════════════════");
        println!("ТАБЛИЦА ИДЕНТИФИКАТОРОВ:");
        println!("═══════════════════════════════════════");
        for (idx, name) in id_list {
            println!("{}: {}", idx, name);
        }
        println!();
    }

    // Output constant table
    let const_table = lexer.get_constant_table();
    let const_list = const_table.list();
    
    if !const_list.is_empty() {
        println!("═══════════════════════════════════════");
        println!("ТАБЛИЦА КОНСТАНТ:");
        println!("═══════════════════════════════════════");
        for (idx, const_val) in const_list {
            println!("{}: {} ({})", idx, const_val.value_string(), const_val.type_name());
        }
        println!();
    }
}

fn format_token(token: &token::Token) -> String {
    use crate::token::TokenType;
    
    match &token.token_type {
        TokenType::Keyword(kw) => format!("Ключевое слово \"{}\"", kw),
        TokenType::Identifier(id) => format!("Идентификатор \"{}\"", id),
        TokenType::IntLiteral(v) => format!("Числовая константа \"{}\"", v),
        TokenType::HexLiteral(v) => format!("Шестнадцатеричная константа \"{}\"", v),
        TokenType::FloatLiteral(v) => format!("Константа с плавающей точкой \"{}\"", v),
        TokenType::StringLiteral(v) => {
            let escaped = v.replace("\\", "\\\\")
                           .replace("\"", "\\\"")
                           .replace("\n", "\\n")
                           .replace("\t", "\\t")
                           .replace("\r", "\\r");
            format!("Строковый литерал \"{}\"", escaped)
        }
        TokenType::CharLiteral(v) => {
            let escaped = v.replace("\\", "\\\\")
                          .replace("'", "\\'")
                          .replace("\n", "\\n")
                          .replace("\t", "\\t")
                          .replace("\r", "\\r");
            format!("Символьный литерал '{}'", escaped)
        }
        TokenType::Plus => "Оператор \"+\"".to_string(),
        TokenType::Minus => "Оператор \"-\"".to_string(),
        TokenType::Star => "Оператор \"*\"".to_string(),
        TokenType::Slash => "Оператор \"/\"".to_string(),
        TokenType::Percent => "Оператор \"%\"".to_string(),
        TokenType::Caret => "Оператор \"^\"".to_string(),
        TokenType::Equal => "Оператор \"=\"".to_string(),
        TokenType::EqualEqual => "Оператор \"==\"".to_string(),
        TokenType::NotEqual => "Оператор \"/=\"".to_string(),
        TokenType::Less => "Оператор \"<\"".to_string(),
        TokenType::Greater => "Оператор \">\"".to_string(),
        TokenType::LessEqual => "Оператор \"<=\"".to_string(),
        TokenType::GreaterEqual => "Оператор \">=\"".to_string(),
        TokenType::AmpAmp => "Оператор \"&&\"".to_string(),
        TokenType::PipePipe => "Оператор \"||\"".to_string(),
        TokenType::Bang => "Оператор \"!\"".to_string(),
        TokenType::Colon => "Оператор \":\"".to_string(),
        TokenType::ColonColon => "Оператор \"::\"".to_string(),
        TokenType::Arrow => "Оператор \"->\"".to_string(),
        TokenType::FatArrow => "Оператор \"=>\"".to_string(),
        TokenType::Backslash => "Оператор \"\\\"".to_string(),
        TokenType::Pipe => "Оператор \"|\"".to_string(),
        TokenType::At => "Оператор \"@\"".to_string(),
        TokenType::Question => "Оператор \"?\"".to_string(),
        TokenType::DotDot => "Оператор \"..\"".to_string(),
        TokenType::DotDotDot => "Оператор \"...\"".to_string(),
        TokenType::DollarSign => "Оператор \"$\"".to_string(),
        TokenType::Tilde => "Оператор \"~\"".to_string(),
        TokenType::ColonGreater => "Оператор \":>\"".to_string(),
        TokenType::LessColon => "Оператор \"<:\"".to_string(),
        TokenType::LeftParen => "Разделитель \"(\"".to_string(),
        TokenType::RightParen => "Разделитель \")\"".to_string(),
        TokenType::LeftBracket => "Разделитель \"[\"".to_string(),
        TokenType::RightBracket => "Разделитель \"]\"".to_string(),
        TokenType::LeftBrace => "Разделитель \"{\"".to_string(),
        TokenType::RightBrace => "Разделитель \"}\"".to_string(),
        TokenType::Comma => "Разделитель \",\"".to_string(),
        TokenType::Semicolon => "Разделитель \";\"".to_string(),
        TokenType::Dot => "Разделитель \".\"".to_string(),
        TokenType::Backtick => "Разделитель \"`\"".to_string(),
        TokenType::Eof => "Конец файла".to_string(),
    }
}
