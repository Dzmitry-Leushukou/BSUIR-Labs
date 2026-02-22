use crate::token::{LexicalError, Token, TokenType};
use crate::symbol_table::{ConstantTable, ConstantValue, IdentifierTable};
use std::str::Chars;
use std::iter::Peekable;

pub struct Lexer<'a> {
    input: Peekable<Chars<'a>>,
    line: usize,
    column: usize,
    current_char: Option<char>,
    identifier_table: IdentifierTable,
    constant_table: ConstantTable,
    errors: Vec<LexicalError>,
}

impl<'a> Lexer<'a> {
    pub fn new(input: &'a str) -> Self {
        let mut input_iter = input.chars().peekable();
        let current_char = input_iter.next();
        
        Lexer {
            input: input_iter,
            line: 1,
            column: 1,
            current_char,
            identifier_table: IdentifierTable::new(),
            constant_table: ConstantTable::new(),
            errors: Vec::new(),
        }
    }

    /// Get the current character without advancing
    fn peek(&mut self) -> Option<char> {
        self.input.peek().copied()
    }

    /// Advance to the next character
    fn advance(&mut self) {
        if let Some(ch) = self.current_char {
            if ch == '\n' {
                self.line += 1;
                self.column = 1;
            } else {
                self.column += 1;
            }
        }
        self.current_char = self.input.next();
    }

    /// Skip whitespace
    fn skip_whitespace(&mut self) {
        while let Some(ch) = self.current_char {
            if ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' {
                self.advance();
            } else {
                break;
            }
        }
    }

    /// Skip a single-line comment (--)
    fn skip_line_comment(&mut self) {
        // Consume the second '-'
        self.advance();
        
        // Skip until newline or EOF
        while let Some(ch) = self.current_char {
            if ch == '\n' {
                self.advance();
                break;
            }
            self.advance();
        }
    }

    /// Skip a multi-line comment with nesting support
    fn skip_multi_comment(&mut self) -> Result<(), LexicalError> {
        let start_line = self.line;
        let start_column = self.column;
        
        // Consume the '-'
        self.advance();
        
        let mut nesting_level = 1;
        
        while let Some(ch) = self.current_char {
            if ch == '{' && self.peek() == Some('-') {
                nesting_level += 1;
                self.advance();
                self.advance();
            } else if ch == '-' && self.peek() == Some('}') {
                nesting_level -= 1;
                self.advance();
                self.advance();
                if nesting_level == 0 {
                    return Ok(());
                }
            } else {
                self.advance();
            }
        }
        
        Err(LexicalError {
            line: start_line,
            column: start_column,
            message: "unclosed multi-line comment".to_string(),
        })
    }

    /// Parse a string literal
    fn read_string(&mut self, start_line: usize, start_column: usize) -> Result<String, LexicalError> {
        self.advance(); // Skip opening quote
        let mut result = String::new();
        
        while let Some(ch) = self.current_char {
            if ch == '"' {
                self.advance(); // Skip closing quote
                return Ok(result);
            } else if ch == '\n' {
                return Err(LexicalError {
                    line: self.line,
                    column: self.column,
                    message: "unclosed string literal (newline not allowed)".to_string(),
                });
            } else if ch == '\\' {
                self.advance();
                match self.current_char {
                    Some('n') => {
                        result.push('\n');
                        self.advance();
                    }
                    Some('t') => {
                        result.push('\t');
                        self.advance();
                    }
                    Some('r') => {
                        result.push('\r');
                        self.advance();
                    }
                    Some('\\') => {
                        result.push('\\');
                        self.advance();
                    }
                    Some('"') => {
                        result.push('"');
                        self.advance();
                    }
                    Some(c) => {
                        result.push('\\');
                        result.push(c);
                        self.advance();
                    }
                    None => {
                        return Err(LexicalError {
                            line: self.line,
                            column: self.column,
                            message: "unclosed string literal (EOF reached)".to_string(),
                        });
                    }
                }
            } else {
                result.push(ch);
                self.advance();
            }
        }
        
        Err(LexicalError {
            line: start_line,
            column: start_column,
            message: "unclosed string literal (EOF reached)".to_string(),
        })
    }

    /// Parse a character literal
    fn read_char(&mut self, start_line: usize, start_column: usize) -> Result<String, LexicalError> {
        self.advance(); // Skip opening quote
        let mut result = String::new();
        
        while let Some(ch) = self.current_char {
            if ch == '\'' {
                self.advance(); // Skip closing quote
                if result.is_empty() {
                    return Err(LexicalError {
                        line: start_line,
                        column: start_column,
                        message: "empty character literal".to_string(),
                    });
                }
                return Ok(result);
            } else if ch == '\n' {
                return Err(LexicalError {
                    line: self.line,
                    column: self.column,
                    message: "unclosed character literal (newline not allowed)".to_string(),
                });
            } else if ch == '\\' {
                self.advance();
                match self.current_char {
                    Some('n') => {
                        result.push('\n');
                        self.advance();
                    }
                    Some('t') => {
                        result.push('\t');
                        self.advance();
                    }
                    Some('r') => {
                        result.push('\r');
                        self.advance();
                    }
                    Some('\\') => {
                        result.push('\\');
                        self.advance();
                    }
                    Some('\'') => {
                        result.push('\'');
                        self.advance();
                    }
                    Some(c) => {
                        result.push('\\');
                        result.push(c);
                        self.advance();
                    }
                    None => {
                        return Err(LexicalError {
                            line: self.line,
                            column: self.column,
                            message: "unclosed character literal (EOF reached)".to_string(),
                        });
                    }
                }
            } else {
                result.push(ch);
                self.advance();
            }
        }
        
        Err(LexicalError {
            line: start_line,
            column: start_column,
            message: "unclosed character literal (EOF reached)".to_string(),
        })
    }

    /// Check if a character is valid for identifiers
    fn is_alnum(&self, ch: char) -> bool {
        ch.is_ascii_alphanumeric() || ch == '_'
    }

    /// Read an identifier or keyword
    fn read_identifier(&mut self) -> String {
        let mut result = String::new();
        
        while let Some(ch) = self.current_char {
            if ch.is_ascii_alphabetic() || ch.is_ascii_digit() || ch == '_' {
                result.push(ch);
                self.advance();
            } else {
                break;
            }
        }
        
        result
    }

    /// Check if a string is a keyword
    fn is_keyword(s: &str) -> bool {
        matches!(
            s,
            "module" | "import" | "data" | "where" | "let" | "in" | "if" | "then" | "else"
                | "case" | "of" | "do" | "record" | "interface" | "implementation" | "forall"
                | "with" | "mutual" | "total" | "partial" | "export" | "private" | "public"
                | "parameters" | "using" | "namespace"
        )
    }

    /// Parse a number (decimal, hex, or float)
    fn read_number(&mut self, start_line: usize, start_column: usize) -> Result<TokenType, LexicalError> {
        let mut num_str = String::new();
        let mut is_hex = false;
        let mut is_float = false;

        // Check for 0x prefix
        if self.current_char == Some('0') {
            num_str.push('0');
            self.advance();
            
            if self.current_char == Some('x') || self.current_char == Some('X') {
                is_hex = true;
                num_str.push('x');
                self.advance();
                
                // Read hex digits
                let start_len = num_str.len();
                while let Some(ch) = self.current_char {
                    if ch.is_ascii_hexdigit() {
                        num_str.push(ch);
                        self.advance();
                    } else {
                        break;
                    }
                }
                
                if num_str.len() == start_len {
                    return Err(LexicalError {
                        line: start_line,
                        column: start_column,
                        message: "empty hexadecimal constant".to_string(),
                    });
                }
                
                if let Ok(_val) = u64::from_str_radix(&num_str[2..], 16) {
                    return Ok(TokenType::HexLiteral(num_str));
                } else {
                    return Err(LexicalError {
                        line: start_line,
                        column: start_column,
                        message: "invalid hexadecimal constant".to_string(),
                    });
                }
            }
        }

        // Read decimal part or continue from first digit
        if !is_hex {
            if num_str.is_empty() {
                while let Some(ch) = self.current_char {
                    if ch.is_ascii_digit() {
                        num_str.push(ch);
                        self.advance();
                    } else {
                        break;
                    }
                }
            }

            // Check for dot (float)
            if self.current_char == Some('.') && self.peek() != Some('.') && self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) {
                is_float = true;
                num_str.push('.');
                self.advance();
                
                // Read fractional part
                while let Some(ch) = self.current_char {
                    if ch.is_ascii_digit() {
                        num_str.push(ch);
                        self.advance();
                    } else {
                        break;
                    }
                }
            }

            // Check for exponent
            if (self.current_char == Some('e') || self.current_char == Some('E')) && is_float {
                num_str.push(self.current_char.unwrap());
                self.advance();
                
                // Optional sign
                if self.current_char == Some('+') || self.current_char == Some('-') {
                    num_str.push(self.current_char.unwrap());
                    self.advance();
                }
                
                // Exponent digits
                let exp_start_len = num_str.len();
                while let Some(ch) = self.current_char {
                    if ch.is_ascii_digit() {
                        num_str.push(ch);
                        self.advance();
                    } else {
                        break;
                    }
                }
                
                if num_str.len() == exp_start_len {
                    return Err(LexicalError {
                        line: start_line,
                        column: start_column,
                        message: "invalid exponent in float literal".to_string(),
                    });
                }
            }
        }

        // Determine token type
        if is_float {
            Ok(TokenType::FloatLiteral(num_str))
        } else if is_hex {
            Ok(TokenType::HexLiteral(num_str))
        } else {
            Ok(TokenType::IntLiteral(num_str))
        }
    }

    /// Get the next token
    pub fn next_token(&mut self) -> Option<Result<Token, LexicalError>> {
        self.skip_whitespace();

        if self.current_char.is_none() {
            return Some(Ok(Token {
                token_type: TokenType::Eof,
                line: self.line,
                column: self.column,
                table_index: None,
            }));
        }

        let line = self.line;
        let column = self.column;
        let ch = self.current_char.unwrap();

        // Handle comments
        if ch == '-' && self.peek() == Some('-') {
            self.skip_line_comment();
            return self.next_token();
        }

        if ch == '{' && self.peek() == Some('-') {
            match self.skip_multi_comment() {
                Ok(_) => return self.next_token(),
                Err(e) => {
                    self.errors.push(e.clone());
                    return Some(Err(e));
                }
            }
        }

        // Handle strings
        if ch == '"' {
            match self.read_string(line, column) {
                Ok(string_val) => {
                    let const_val = ConstantValue::String(string_val.clone());
                    let idx = self.constant_table.get_or_insert(const_val);
                    return Some(Ok(Token {
                        token_type: TokenType::StringLiteral(string_val),
                        line,
                        column,
                        table_index: Some(idx),
                    }));
                }
                Err(e) => {
                    self.errors.push(e.clone());
                    return Some(Err(e));
                }
            }
        }

        // Handle characters
        if ch == '\'' {
            match self.read_char(line, column) {
                Ok(char_val) => {
                    let const_val = ConstantValue::Char(char_val.clone());
                    let idx = self.constant_table.get_or_insert(const_val);
                    return Some(Ok(Token {
                        token_type: TokenType::CharLiteral(char_val),
                        line,
                        column,
                        table_index: Some(idx),
                    }));
                }
                Err(e) => {
                    self.errors.push(e.clone());
                    return Some(Err(e));
                }
            }
        }

        // Handle identifiers and numbers
        if ch.is_ascii_alphabetic() {
            let ident = self.read_identifier();
            if Self::is_keyword(&ident) {
                return Some(Ok(Token {
                    token_type: TokenType::Keyword(ident),
                    line,
                    column,
                    table_index: None,
                }));
            } else {
                let idx = self.identifier_table.get_or_insert(&ident);
                return Some(Ok(Token {
                    token_type: TokenType::Identifier(ident),
                    line,
                    column,
                    table_index: Some(idx),
                }));
            }
        }

        if ch.is_ascii_digit() {
            match self.read_number(line, column) {
                Ok(token_type) => {
                    let const_val = match &token_type {
                        TokenType::IntLiteral(s) => {
                            if let Ok(val) = s.parse::<i64>() {
                                ConstantValue::Int(val)
                            } else {
                                return Some(Err(LexicalError {
                                    line,
                                    column,
                                    message: "integer overflow".to_string(),
                                }));
                            }
                        }
                        TokenType::HexLiteral(s) => {
                            if let Ok(val) = u64::from_str_radix(&s[2..], 16) {
                                ConstantValue::Hex(val)
                            } else {
                                return Some(Err(LexicalError {
                                    line,
                                    column,
                                    message: "invalid hexadecimal literal".to_string(),
                                }));
                            }
                        }
                        TokenType::FloatLiteral(s) => ConstantValue::Float(s.clone()),
                        _ => unreachable!(),
                    };
                    let idx = self.constant_table.get_or_insert(const_val);
                    return Some(Ok(Token {
                        token_type,
                        line,
                        column,
                        table_index: Some(idx),
                    }));
                }
                Err(e) => {
                    self.errors.push(e.clone());
                    return Some(Err(e));
                }
            }
        }

        // Handle operators and delimiters
        let token_type = match ch {
            '+' => {
                self.advance();
                TokenType::Plus
            }
            '-' => {
                self.advance();
                if self.current_char == Some('>') {
                    self.advance();
                    TokenType::Arrow
                } else {
                    TokenType::Minus
                }
            }
            '*' => {
                self.advance();
                TokenType::Star
            }
            '/' => {
                self.advance();
                TokenType::Slash
            }
            '%' => {
                self.advance();
                TokenType::Percent
            }
            '^' => {
                self.advance();
                TokenType::Caret
            }
            '=' => {
                self.advance();
                if self.current_char == Some('=') {
                    self.advance();
                    TokenType::EqualEqual
                } else if self.current_char == Some('>') {
                    self.advance();
                    TokenType::FatArrow
                } else {
                    TokenType::Equal
                }
            }
            '!' => {
                self.advance();
                if self.current_char == Some('=') {
                    self.advance();
                    TokenType::NotEqual
                } else {
                    TokenType::Bang
                }
            }
            '<' => {
                self.advance();
                if self.current_char == Some('=') {
                    self.advance();
                    TokenType::LessEqual
                } else if self.current_char == Some(':') {
                    self.advance();
                    TokenType::LessColon
                } else {
                    TokenType::Less
                }
            }
            '>' => {
                self.advance();
                if self.current_char == Some('=') {
                    self.advance();
                    TokenType::GreaterEqual
                } else {
                    TokenType::Greater
                }
            }
            '&' => {
                self.advance();
                if self.current_char == Some('&') {
                    self.advance();
                    TokenType::AmpAmp
                } else {
                    let e = LexicalError {
                        line,
                        column,
                        message: format!("unrecognized character '{}'", ch),
                    };
                    self.errors.push(e.clone());
                    return Some(Err(e));
                }
            }
            '|' => {
                self.advance();
                if self.current_char == Some('|') {
                    self.advance();
                    TokenType::PipePipe
                } else {
                    TokenType::Pipe
                }
            }
            ':' => {
                self.advance();
                if self.current_char == Some(':') {
                    self.advance();
                    TokenType::ColonColon
                } else if self.current_char == Some('>') {
                    self.advance();
                    TokenType::ColonGreater
                } else {
                    TokenType::Colon
                }
            }
            '\\' => {
                self.advance();
                TokenType::Backslash
            }
            '@' => {
                self.advance();
                TokenType::At
            }
            '?' => {
                self.advance();
                TokenType::Question
            }
            '$' => {
                self.advance();
                TokenType::DollarSign
            }
            '~' => {
                self.advance();
                TokenType::Tilde
            }
            '.' => {
                self.advance();
                if self.current_char == Some('.') {
                    self.advance();
                    if self.current_char == Some('.') {
                        self.advance();
                        TokenType::DotDotDot
                    } else {
                        TokenType::DotDot
                    }
                } else {
                    TokenType::Dot
                }
            }
            '(' => {
                self.advance();
                TokenType::LeftParen
            }
            ')' => {
                self.advance();
                TokenType::RightParen
            }
            '[' => {
                self.advance();
                TokenType::LeftBracket
            }
            ']' => {
                self.advance();
                TokenType::RightBracket
            }
            '{' => {
                self.advance();
                TokenType::LeftBrace
            }
            '}' => {
                self.advance();
                TokenType::RightBrace
            }
            ',' => {
                self.advance();
                TokenType::Comma
            }
            ';' => {
                self.advance();
                TokenType::Semicolon
            }
            '`' => {
                self.advance();
                TokenType::Backtick
            }
            _ => {
                self.advance();
                let e = LexicalError {
                    line,
                    column,
                    message: format!("unrecognized character '{}'", ch),
                };
                self.errors.push(e.clone());
                return Some(Err(e));
            }
        };

        Some(Ok(Token {
            token_type,
            line,
            column,
            table_index: None,
        }))
    }

    /// Get all tokens
    pub fn tokenize(&mut self) -> (Vec<Token>, Vec<LexicalError>) {
        let mut tokens = Vec::new();
        
        loop {
            match self.next_token() {
                Some(Ok(token)) => {
                    if token.token_type == TokenType::Eof {
                        tokens.push(token);
                        break;
                    }
                    tokens.push(token);
                }
                Some(Err(_)) => {
                    // Error already pushed to self.errors
                    // Continue tokenizing to find all errors
                }
                None => break,
            }
        }

        (tokens, self.errors.clone())
    }

    pub fn get_identifier_table(&self) -> &IdentifierTable {
        &self.identifier_table
    }

    pub fn get_constant_table(&self) -> &ConstantTable {
        &self.constant_table
    }
}
