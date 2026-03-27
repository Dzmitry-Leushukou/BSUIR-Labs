mod lexer;
mod symbol_table;
mod token;

use std::env;
use std::fs;
use std::path::Path;
use std::process;

use lexer::Lexer;
use token::TokenType;

#[derive(Debug, Clone)]
enum Expr {
    Identifier(String),
    Literal(String),
    Unary {
        op: String,
        expr: Box<Expr>,
    },
    Binary {
        op: String,
        left: Box<Expr>,
        right: Box<Expr>,
    },
    Apply {
        func: Box<Expr>,
        arg: Box<Expr>,
    },
    Lambda {
        params: Vec<String>,
        body: Box<Expr>,
    },
    List(Vec<Expr>),
    OperatorRef(String),
    SectionRight {
        op: String,
        rhs: Box<Expr>,
    },
}

#[derive(Debug, Clone)]
struct ParseError {
    column: usize,
    message: String,
}

#[derive(Debug, Clone)]
enum TokenKind {
    Ident(String),
    Number(String),
    StringLit(String),
    CharLit(String),
    LParen,
    RParen,
    LBracket,
    RBracket,
    Comma,
    Backslash,
    Backtick,
    FatArrow,
    Operator(String),
}

#[derive(Debug, Clone)]
struct Token {
    kind: TokenKind,
    column: usize,
}

#[derive(Debug)]
struct ParsedAssignment {
    line: usize,
    lhs: String,
    rhs: String,
    tree: Expr,
}

struct Parser {
    tokens: Vec<Token>,
    pos: usize,
}

impl Parser {
    fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, pos: 0 }
    }

    fn parse(mut self) -> Result<Expr, ParseError> {
        if self.tokens.is_empty() {
            return Err(ParseError {
                column: 1,
                message: "пустое выражение".to_string(),
            });
        }

        let expr = self.parse_expr(0)?;
        if let Some(token) = self.peek() {
            return Err(ParseError {
                column: token.column,
                message: "неожиданный токен после корректного фрагмента".to_string(),
            });
        }

        Ok(expr)
    }

    fn parse_expr(&mut self, min_bp: u8) -> Result<Expr, ParseError> {
        let mut lhs = self.parse_prefix()?;

        loop {
            if let Some((name, l_bp, r_bp)) = self.peek_backtick_operator() {
                if l_bp < min_bp {
                    break;
                }
                self.pos += 3;
                let rhs = self.parse_expr(r_bp)?;
                lhs = Expr::Binary {
                    op: name,
                    left: Box::new(lhs),
                    right: Box::new(rhs),
                };
                continue;
            }

            let Some((op, l_bp, r_bp)) = self.peek_infix_operator() else {
                if self.can_start_expr() {
                    let l_bp = 90;
                    let r_bp = 91;
                    if l_bp < min_bp {
                        break;
                    }
                    let rhs = self.parse_expr(r_bp)?;
                    lhs = Expr::Apply {
                        func: Box::new(lhs),
                        arg: Box::new(rhs),
                    };
                    continue;
                }
                break;
            };

            if l_bp < min_bp {
                break;
            }

            self.pos += 1;
            let rhs = self.parse_expr(r_bp)?;
            lhs = Expr::Binary {
                op,
                left: Box::new(lhs),
                right: Box::new(rhs),
            };
        }

        Ok(lhs)
    }

    fn parse_prefix(&mut self) -> Result<Expr, ParseError> {
        let token = self.next().cloned().ok_or(ParseError {
            column: self.current_column(),
            message: "ожидалось выражение".to_string(),
        })?;

        match token.kind {
            TokenKind::Ident(name) => Ok(Expr::Identifier(name)),
            TokenKind::Number(value) => Ok(Expr::Literal(value)),
            TokenKind::StringLit(value) => Ok(Expr::Literal(value)),
            TokenKind::CharLit(value) => Ok(Expr::Literal(value)),
            TokenKind::LBracket => self.parse_list(token.column),
            TokenKind::LParen => self.parse_parenthesized(token.column),
            TokenKind::Backslash => self.parse_lambda(token.column),
            TokenKind::Operator(op) if op == "+" || op == "-" || op == "!" || op == "~" => {
                let rhs = self.parse_expr(85)?;
                Ok(Expr::Unary {
                    op,
                    expr: Box::new(rhs),
                })
            }
            _ => Err(ParseError {
                column: token.column,
                message: "неожиданное начало выражения".to_string(),
            }),
        }
    }

    fn parse_list(&mut self, start_column: usize) -> Result<Expr, ParseError> {
        if matches!(self.peek_kind(), Some(TokenKind::RBracket)) {
            self.pos += 1;
            return Ok(Expr::List(Vec::new()));
        }

        let mut elements = Vec::new();
        loop {
            let value = self.parse_expr(0)?;
            elements.push(value);

            match self.peek_kind() {
                Some(TokenKind::Comma) => {
                    self.pos += 1;
                }
                Some(TokenKind::RBracket) => {
                    self.pos += 1;
                    break;
                }
                Some(token) => {
                    return Err(ParseError {
                        column: self.peek().map(|t| t.column).unwrap_or(start_column),
                        message: format!("ожидалась ',' или ']', получено {}", token_name(token)),
                    });
                }
                None => {
                    return Err(ParseError {
                        column: start_column,
                        message: "не закрыт список ']'".to_string(),
                    });
                }
            }
        }

        Ok(Expr::List(elements))
    }

    fn parse_parenthesized(&mut self, start_column: usize) -> Result<Expr, ParseError> {
        if matches!(self.peek_kind(), Some(TokenKind::Operator(_))) {
            let op_token = self.next().cloned().ok_or(ParseError {
                column: start_column,
                message: "ожидался оператор".to_string(),
            })?;

            let TokenKind::Operator(op_name) = op_token.kind else {
                return Err(ParseError {
                    column: op_token.column,
                    message: "ожидался оператор".to_string(),
                });
            };

            if matches!(self.peek_kind(), Some(TokenKind::RParen)) {
                self.pos += 1;
                return Ok(Expr::OperatorRef(op_name));
            }

            let rhs = self.parse_expr(0)?;
            self.expect_rparen(start_column)?;
            return Ok(Expr::SectionRight {
                op: op_name,
                rhs: Box::new(rhs),
            });
        }

        if matches!(self.peek_kind(), Some(TokenKind::RParen)) {
            return Err(ParseError {
                column: self.peek().map(|t| t.column).unwrap_or(start_column),
                message: "пустые скобки не поддерживаются".to_string(),
            });
        }

        let expr = self.parse_expr(0)?;
        self.expect_rparen(start_column)?;
        Ok(expr)
    }

    fn parse_lambda(&mut self, start_column: usize) -> Result<Expr, ParseError> {
        let mut params = Vec::new();

        loop {
            let Some(token) = self.peek() else {
                return Err(ParseError {
                    column: start_column,
                    message: "после '\\' ожидаются параметры и '=>'".to_string(),
                });
            };

            match &token.kind {
                TokenKind::Ident(name) => {
                    params.push(name.clone());
                    self.pos += 1;
                }
                TokenKind::FatArrow => {
                    self.pos += 1;
                    break;
                }
                _ => {
                    return Err(ParseError {
                        column: token.column,
                        message: "в лямбде ожидался параметр или '=>'".to_string(),
                    });
                }
            }
        }

        if params.is_empty() {
            return Err(ParseError {
                column: start_column,
                message: "в лямбде не указан параметр".to_string(),
            });
        }

        let body = self.parse_expr(0)?;
        Ok(Expr::Lambda {
            params,
            body: Box::new(body),
        })
    }

    fn expect_rparen(&mut self, start_column: usize) -> Result<(), ParseError> {
        match self.next() {
            Some(Token {
                kind: TokenKind::RParen,
                ..
            }) => Ok(()),
            Some(token) => Err(ParseError {
                column: token.column,
                message: "ожидалась ')'".to_string(),
            }),
            None => Err(ParseError {
                column: start_column,
                message: "не закрыта скобка ')'".to_string(),
            }),
        }
    }

    fn peek_backtick_operator(&self) -> Option<(String, u8, u8)> {
        if self.pos + 2 >= self.tokens.len() {
            return None;
        }

        let a = &self.tokens[self.pos].kind;
        let b = &self.tokens[self.pos + 1].kind;
        let c = &self.tokens[self.pos + 2].kind;

        match (a, b, c) {
            (TokenKind::Backtick, TokenKind::Ident(name), TokenKind::Backtick) => {
                let (l_bp, r_bp) = infix_binding_power("`")?;
                Some((name.clone(), l_bp, r_bp))
            }
            _ => None,
        }
    }

    fn peek_infix_operator(&self) -> Option<(String, u8, u8)> {
        let token = self.peek()?;
        let TokenKind::Operator(op) = &token.kind else {
            return None;
        };

        let (l_bp, r_bp) = infix_binding_power(op)?;
        Some((op.clone(), l_bp, r_bp))
    }

    fn can_start_expr(&self) -> bool {
        match self.peek_kind() {
            Some(TokenKind::Ident(_))
            | Some(TokenKind::Number(_))
            | Some(TokenKind::StringLit(_))
            | Some(TokenKind::CharLit(_))
            | Some(TokenKind::LParen)
            | Some(TokenKind::LBracket)
            | Some(TokenKind::Backslash) => true,
            Some(TokenKind::Operator(op)) => op == "+" || op == "-" || op == "!" || op == "~",
            _ => false,
        }
    }

    fn current_column(&self) -> usize {
        self.peek().map(|t| t.column).unwrap_or(1)
    }

    fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.pos)
    }

    fn next(&mut self) -> Option<&Token> {
        let token = self.tokens.get(self.pos);
        if token.is_some() {
            self.pos += 1;
        }
        token
    }

    fn peek_kind(&self) -> Option<&TokenKind> {
        self.peek().map(|t| &t.kind)
    }
}

fn token_name(token: &TokenKind) -> String {
    match token {
        TokenKind::Ident(v) => format!("идентификатор '{}'", v),
        TokenKind::Number(v) => format!("число '{}'", v),
        TokenKind::StringLit(_) => "строка".to_string(),
        TokenKind::CharLit(_) => "символ".to_string(),
        TokenKind::LParen => "'('".to_string(),
        TokenKind::RParen => "')'".to_string(),
        TokenKind::LBracket => "'['".to_string(),
        TokenKind::RBracket => "']'".to_string(),
        TokenKind::Comma => "','".to_string(),
        TokenKind::Backslash => "'\\'".to_string(),
        TokenKind::Backtick => "'`'".to_string(),
        TokenKind::FatArrow => "'=>'".to_string(),
        TokenKind::Operator(op) => format!("оператор '{}'", op),
    }
}

fn infix_binding_power(op: &str) -> Option<(u8, u8)> {
    let binding = match op {
        "^" => (80, 79),
        "*" | "/" | "%" => (70, 71),
        "`" => (70, 71),
        "+" | "-" | "++" => (60, 61),
        "==" | "/=" | "<" | ">" | "<=" | ">=" => (50, 51),
        "&&" => (40, 41),
        "||" => (35, 36),
        "::" => (30, 29),
        "$" => (20, 19),
        "=" => (10, 9),
        _ => return None,
    };
    Some(binding)
}

fn looks_like_type_signature(lhs: &str) -> bool {
    let mut lexer = Lexer::new(lhs);
    let (tokens, errors) = lexer.tokenize();
    if !errors.is_empty() {
        return lhs.contains(':') && !lhs.contains("::");
    }
    tokens
        .iter()
        .any(|token| matches!(token.token_type, TokenType::Colon))
}

fn escape_string(value: &str) -> String {
    value
        .replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\t', "\\t")
        .replace('\r', "\\r")
}

fn escape_char(value: &str) -> String {
    value
        .replace('\\', "\\\\")
        .replace('\'', "\\'")
        .replace('\n', "\\n")
        .replace('\t', "\\t")
        .replace('\r', "\\r")
}

fn lex_token_name(token_type: &TokenType) -> String {
    match token_type {
        TokenType::Keyword(v) => format!("keyword '{}'", v),
        TokenType::Identifier(v) => format!("identifier '{}'", v),
        TokenType::IntLiteral(v) => format!("int '{}'", v),
        TokenType::HexLiteral(v) => format!("hex '{}'", v),
        TokenType::FloatLiteral(v) => format!("float '{}'", v),
        TokenType::StringLiteral(_) => "string".to_string(),
        TokenType::CharLiteral(_) => "char".to_string(),
        TokenType::Plus => "'+'".to_string(),
        TokenType::Minus => "'-'".to_string(),
        TokenType::Star => "'*'".to_string(),
        TokenType::Slash => "'/'".to_string(),
        TokenType::Percent => "'%'".to_string(),
        TokenType::Caret => "'^'".to_string(),
        TokenType::Equal => "'='".to_string(),
        TokenType::EqualEqual => "'=='".to_string(),
        TokenType::NotEqual => "'/='".to_string(),
        TokenType::Less => "'<'".to_string(),
        TokenType::Greater => "'>'".to_string(),
        TokenType::LessEqual => "'<='".to_string(),
        TokenType::GreaterEqual => "'>='".to_string(),
        TokenType::AmpAmp => "'&&'".to_string(),
        TokenType::PipePipe => "'||'".to_string(),
        TokenType::Bang => "'!'".to_string(),
        TokenType::Colon => "':'".to_string(),
        TokenType::ColonColon => "'::'".to_string(),
        TokenType::Arrow => "'->'".to_string(),
        TokenType::FatArrow => "'=>'".to_string(),
        TokenType::Backslash => "'\\'".to_string(),
        TokenType::Pipe => "'|'".to_string(),
        TokenType::At => "'@'".to_string(),
        TokenType::Question => "'?'".to_string(),
        TokenType::DotDot => "'..'".to_string(),
        TokenType::DotDotDot => "'...'".to_string(),
        TokenType::DollarSign => "'$'".to_string(),
        TokenType::Tilde => "'~'".to_string(),
        TokenType::ColonGreater => "':>'".to_string(),
        TokenType::LessColon => "'<:'".to_string(),
        TokenType::LeftParen => "'('".to_string(),
        TokenType::RightParen => "')'".to_string(),
        TokenType::LeftBracket => "'['".to_string(),
        TokenType::RightBracket => "']'".to_string(),
        TokenType::LeftBrace => "'{'".to_string(),
        TokenType::RightBrace => "'}'".to_string(),
        TokenType::Comma => "','".to_string(),
        TokenType::Semicolon => "';'".to_string(),
        TokenType::Dot => "'.'".to_string(),
        TokenType::Backtick => "'`'".to_string(),
        TokenType::Eof => "EOF".to_string(),
    }
}

fn map_lex_token(
    token: crate::token::Token,
    base_column: usize,
) -> Result<Option<Token>, ParseError> {
    let column = base_column + token.column.saturating_sub(1);
    let kind = match token.token_type {
        TokenType::Keyword(value) | TokenType::Identifier(value) => TokenKind::Ident(value),
        TokenType::IntLiteral(value)
        | TokenType::HexLiteral(value)
        | TokenType::FloatLiteral(value) => TokenKind::Number(value),
        TokenType::StringLiteral(value) => {
            TokenKind::StringLit(format!("\"{}\"", escape_string(&value)))
        }
        TokenType::CharLiteral(value) => TokenKind::CharLit(format!("'{}'", escape_char(&value))),
        TokenType::LeftParen => TokenKind::LParen,
        TokenType::RightParen => TokenKind::RParen,
        TokenType::LeftBracket => TokenKind::LBracket,
        TokenType::RightBracket => TokenKind::RBracket,
        TokenType::Comma => TokenKind::Comma,
        TokenType::Backslash => TokenKind::Backslash,
        TokenType::Backtick => TokenKind::Backtick,
        TokenType::FatArrow => TokenKind::FatArrow,
        TokenType::Plus => TokenKind::Operator("+".to_string()),
        TokenType::Minus => TokenKind::Operator("-".to_string()),
        TokenType::Star => TokenKind::Operator("*".to_string()),
        TokenType::Slash => TokenKind::Operator("/".to_string()),
        TokenType::Percent => TokenKind::Operator("%".to_string()),
        TokenType::Caret => TokenKind::Operator("^".to_string()),
        TokenType::Equal => TokenKind::Operator("=".to_string()),
        TokenType::EqualEqual => TokenKind::Operator("==".to_string()),
        TokenType::NotEqual => TokenKind::Operator("/=".to_string()),
        TokenType::Less => TokenKind::Operator("<".to_string()),
        TokenType::Greater => TokenKind::Operator(">".to_string()),
        TokenType::LessEqual => TokenKind::Operator("<=".to_string()),
        TokenType::GreaterEqual => TokenKind::Operator(">=".to_string()),
        TokenType::AmpAmp => TokenKind::Operator("&&".to_string()),
        TokenType::PipePipe => TokenKind::Operator("||".to_string()),
        TokenType::Bang => TokenKind::Operator("!".to_string()),
        TokenType::Colon => TokenKind::Operator(":".to_string()),
        TokenType::ColonColon => TokenKind::Operator("::".to_string()),
        TokenType::Arrow => TokenKind::Operator("->".to_string()),
        TokenType::Pipe => TokenKind::Operator("|".to_string()),
        TokenType::At => TokenKind::Operator("@".to_string()),
        TokenType::Question => TokenKind::Operator("?".to_string()),
        TokenType::DotDot => TokenKind::Operator("..".to_string()),
        TokenType::DotDotDot => TokenKind::Operator("...".to_string()),
        TokenType::DollarSign => TokenKind::Operator("$".to_string()),
        TokenType::Tilde => TokenKind::Operator("~".to_string()),
        TokenType::ColonGreater => TokenKind::Operator(":>".to_string()),
        TokenType::LessColon => TokenKind::Operator("<:".to_string()),
        TokenType::Dot => TokenKind::Operator(".".to_string()),
        TokenType::Eof => return Ok(None),
        other => {
            return Err(ParseError {
                column,
                message: format!(
                    "токен {} не поддерживается в выражении",
                    lex_token_name(&other)
                ),
            });
        }
    };
    Ok(Some(Token { kind, column }))
}

fn merge_dotted_identifiers(tokens: Vec<Token>) -> Vec<Token> {
    let mut result = Vec::new();
    let mut index = 0;

    while index < tokens.len() {
        if let TokenKind::Ident(mut name) = tokens[index].kind.clone() {
            let start_column = tokens[index].column;
            let mut end_index = index;
            while end_index + 2 < tokens.len() {
                match (&tokens[end_index + 1].kind, &tokens[end_index + 2].kind) {
                    (TokenKind::Operator(op), TokenKind::Ident(next)) if op == "." => {
                        name.push('.');
                        name.push_str(next);
                        end_index += 2;
                    }
                    _ => break,
                }
            }

            if end_index > index {
                result.push(Token {
                    kind: TokenKind::Ident(name),
                    column: start_column,
                });
                index = end_index + 1;
                continue;
            }
        }

        result.push(tokens[index].clone());
        index += 1;
    }

    result
}

fn merge_compound_operators(tokens: Vec<Token>) -> Vec<Token> {
    let mut result = Vec::new();
    let mut index = 0;

    while index < tokens.len() {
        if index + 1 < tokens.len() {
            let first = &tokens[index];
            let second = &tokens[index + 1];

            if let (TokenKind::Operator(a), TokenKind::Operator(b)) = (&first.kind, &second.kind) {
                if a == "+" && b == "+" {
                    result.push(Token {
                        kind: TokenKind::Operator("++".to_string()),
                        column: first.column,
                    });
                    index += 2;
                    continue;
                }
            }
        }

        result.push(tokens[index].clone());
        index += 1;
    }

    result
}

fn tokenize_expression(input: &str, base_column: usize) -> Result<Vec<Token>, ParseError> {
    let mut lexer = Lexer::new(input);
    let (lex_tokens, lex_errors) = lexer.tokenize();

    if let Some(first_error) = lex_errors.first() {
        return Err(ParseError {
            column: base_column + first_error.column.saturating_sub(1),
            message: first_error.message.clone(),
        });
    }

    let mut tokens = Vec::new();
    for lex_token in lex_tokens {
        if let Some(token) = map_lex_token(lex_token, base_column)? {
            tokens.push(token);
        }
    }

    Ok(merge_dotted_identifiers(merge_compound_operators(tokens)))
}

fn strip_line_comment(line: &str) -> String {
    let chars: Vec<(usize, char)> = line.char_indices().collect();
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;

    let mut idx = 0;
    while idx < chars.len() {
        let (byte_idx, ch) = chars[idx];

        if in_string {
            if escaped {
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '"' {
                in_string = false;
            }
            idx += 1;
            continue;
        }

        if in_char {
            if escaped {
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '\'' {
                in_char = false;
            }
            idx += 1;
            continue;
        }

        if ch == '"' {
            in_string = true;
            idx += 1;
            continue;
        }

        if ch == '\'' {
            in_char = true;
            idx += 1;
            continue;
        }

        if ch == '-' && idx + 1 < chars.len() && chars[idx + 1].1 == '-' {
            return line[..byte_idx].to_string();
        }

        idx += 1;
    }

    line.to_string()
}

fn find_assignment(line: &str) -> Option<(String, String, usize)> {
    let chars: Vec<(usize, char)> = line.char_indices().collect();
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;
    let mut paren_depth = 0usize;
    let mut bracket_depth = 0usize;
    let mut brace_depth = 0usize;

    for idx in 0..chars.len() {
        let (byte_idx, ch) = chars[idx];

        if in_string {
            if escaped {
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '"' {
                in_string = false;
            }
            continue;
        }

        if in_char {
            if escaped {
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '\'' {
                in_char = false;
            }
            continue;
        }

        if ch == '"' {
            in_string = true;
            continue;
        }

        if ch == '\'' {
            in_char = true;
            continue;
        }

        match ch {
            '(' => {
                paren_depth += 1;
                continue;
            }
            ')' => {
                paren_depth = paren_depth.saturating_sub(1);
                continue;
            }
            '[' => {
                bracket_depth += 1;
                continue;
            }
            ']' => {
                bracket_depth = bracket_depth.saturating_sub(1);
                continue;
            }
            '{' => {
                brace_depth += 1;
                continue;
            }
            '}' => {
                brace_depth = brace_depth.saturating_sub(1);
                continue;
            }
            _ => {}
        }

        if ch != '=' {
            continue;
        }

        if paren_depth != 0 || bracket_depth != 0 || brace_depth != 0 {
            continue;
        }

        let prev = if idx > 0 {
            Some(chars[idx - 1].1)
        } else {
            None
        };
        let next = if idx + 1 < chars.len() {
            Some(chars[idx + 1].1)
        } else {
            None
        };

        if matches!(next, Some('>') | Some('=')) {
            continue;
        }

        if matches!(
            prev,
            Some('=') | Some('<') | Some('>') | Some('/') | Some('!')
        ) {
            continue;
        }

        let lhs = line[..byte_idx].trim().to_string();
        let rhs_raw = &line[byte_idx + 1..];
        let rhs = rhs_raw.trim().to_string();

        let mut rhs_col = line[..byte_idx].chars().count() + 2;
        for ch2 in rhs_raw.chars() {
            if ch2.is_whitespace() {
                rhs_col += 1;
            } else {
                break;
            }
        }

        return Some((lhs, rhs, rhs_col));
    }

    None
}

fn analyze_file(content: &str) -> (Vec<ParsedAssignment>, Vec<String>) {
    let mut parsed = Vec::new();
    let mut errors = Vec::new();

    for (idx, raw_line) in content.lines().enumerate() {
        let line_number = idx + 1;
        let clean_line = strip_line_comment(raw_line);
        let line = clean_line.trim_end();

        if line.trim().is_empty() {
            continue;
        }

        let Some((lhs, rhs, rhs_col)) = find_assignment(line) else {
            continue;
        };

        if lhs.trim().is_empty() {
            continue;
        }

        let lhs_trimmed_start = lhs.trim_start();
        if lhs_trimmed_start.starts_with("data ")
            || lhs_trimmed_start.starts_with("record ")
            || lhs_trimmed_start.starts_with("interface ")
        {
            continue;
        }

        if looks_like_type_signature(&lhs) {
            continue;
        }

        if rhs.is_empty() {
            errors.push(format!(
                "Строка {}: после '=' отсутствует выражение",
                line_number
            ));
            continue;
        }

        let tokens = match tokenize_expression(&rhs, rhs_col) {
            Ok(t) => t,
            Err(err) => {
                errors.push(format!(
                    "Строка {}: не удалось токенизировать выражение в колонке {}: {}",
                    line_number, err.column, err.message
                ));
                continue;
            }
        };

        let parser = Parser::new(tokens);
        let rhs_tree = match parser.parse() {
            Ok(tree) => tree,
            Err(err) => {
                errors.push(format!(
                    "Строка {}: не удалось разобрать выражение в колонке {}: {}",
                    line_number, err.column, err.message
                ));
                continue;
            }
        };

        let assignment_tree = Expr::Binary {
            op: "=".to_string(),
            left: Box::new(Expr::Identifier(lhs.clone())),
            right: Box::new(rhs_tree),
        };

        parsed.push(ParsedAssignment {
            line: line_number,
            lhs,
            rhs,
            tree: assignment_tree,
        });
    }

    (parsed, errors)
}

fn expr_label(expr: &Expr) -> String {
    match expr {
        Expr::Identifier(name) => name.clone(),
        Expr::Literal(value) => value.clone(),
        Expr::Unary { op, .. } => format!("unary {}", op),
        Expr::Binary { op, .. } => op.clone(),
        Expr::Apply { .. } => "apply".to_string(),
        Expr::Lambda { params, .. } => format!("lambda {}", params.join(" ")),
        Expr::List(_) => "list".to_string(),
        Expr::OperatorRef(op) => format!("operator {}", op),
        Expr::SectionRight { op, .. } => format!("section ({} _)", op),
    }
}

fn expr_children(expr: &Expr) -> Vec<&Expr> {
    match expr {
        Expr::Identifier(_) | Expr::Literal(_) | Expr::OperatorRef(_) => Vec::new(),
        Expr::Unary { expr, .. } => vec![expr],
        Expr::Binary { left, right, .. } => vec![left, right],
        Expr::Apply { func, arg } => vec![func, arg],
        Expr::Lambda { body, .. } => vec![body],
        Expr::List(elements) => elements.iter().collect(),
        Expr::SectionRight { rhs, .. } => vec![rhs],
    }
}

fn render_tree(expr: &Expr) -> String {
    let mut lines = Vec::new();
    lines.push(expr_label(expr));
    render_tree_children(expr, "", &mut lines);
    lines.join("\n")
}

fn render_tree_children(expr: &Expr, prefix: &str, lines: &mut Vec<String>) {
    let children = expr_children(expr);
    for (idx, child) in children.iter().enumerate() {
        let is_last = idx + 1 == children.len();
        let branch = if is_last { "└──" } else { "├──" };
        lines.push(format!("{}{} {}", prefix, branch, expr_label(child)));
        let next_prefix = if is_last {
            format!("{}    ", prefix)
        } else {
            format!("{}│   ", prefix)
        };
        render_tree_children(child, &next_prefix, lines);
    }
}

fn brief(expr: &Expr) -> String {
    match expr {
        Expr::Identifier(v) => v.clone(),
        Expr::Literal(v) => v.clone(),
        Expr::OperatorRef(op) => format!("operator {}", op),
        Expr::Lambda { params, .. } => format!("lambda({})", params.join(",")),
        Expr::List(values) => {
            let items: Vec<String> = values.iter().map(brief).collect();
            format!("[{}]", items.join(", "))
        }
        Expr::SectionRight { op, rhs } => format!("({} {})", op, brief(rhs)),
        Expr::Unary { .. } | Expr::Binary { .. } | Expr::Apply { .. } => "выражение".to_string(),
    }
}

fn step_text_for_binary(op: &str, left: &str, right: &str) -> String {
    match op {
        "+" => format!("прибавить {} к {}", right, left),
        "-" => format!("вычесть {} из {}", right, left),
        "*" => format!("умножить {} и {}", left, right),
        "/" => format!("разделить {} на {}", left, right),
        "%" => format!("взять остаток от деления {} на {}", left, right),
        "^" => format!("возвести {} в степень {}", left, right),
        _ => format!("выполнить '{}' над {} и {}", op, left, right),
    }
}

fn build_steps(expr: &Expr, steps: &mut Vec<String>) -> String {
    match expr {
        Expr::Identifier(name) => name.clone(),
        Expr::Literal(value) => value.clone(),
        Expr::OperatorRef(op) => format!("operator {}", op),
        Expr::SectionRight { op, rhs } => format!("({} {})", op, build_steps(rhs, steps)),
        Expr::List(values) => {
            let parts: Vec<String> = values.iter().map(|v| build_steps(v, steps)).collect();
            format!("[{}]", parts.join(", "))
        }
        Expr::Lambda { params, .. } => format!("lambda({})", params.join(",")),
        Expr::Unary { op, expr } => {
            let value = build_steps(expr, steps);
            let text = format!("применить унарный '{}' к {}", op, value);
            steps.push(text);
            format!("результат ({})", steps.len())
        }
        Expr::Apply { func, arg } => {
            let fn_ref = build_steps(func, steps);
            let arg_ref = build_steps(arg, steps);
            let text = format!("применить {} к {}", fn_ref, arg_ref);
            steps.push(text);
            format!("результат ({})", steps.len())
        }
        Expr::Binary { op, left, right } => {
            if op == "=" {
                let value_ref = build_steps(right, steps);
                let target = brief(left);
                let text = format!("поместить {} в {}", value_ref, target);
                steps.push(text);
                return format!("результат ({})", steps.len());
            }

            let left_ref = build_steps(left, steps);
            let right_ref = build_steps(right, steps);
            let text = step_text_for_binary(op, &left_ref, &right_ref);
            steps.push(text);
            format!("результат ({})", steps.len())
        }
    }
}

fn print_analysis(parsed: &[ParsedAssignment], errors: &[String], file: &str) {
    println!("Файл: {}", file);
    println!("Разобрано присваиваний: {}", parsed.len());
    println!("Неразобранных выражений: {}", errors.len());

    if !errors.is_empty() {
        println!();
        println!("Неразобранные выражения:");
        for err in errors {
            println!("- {}", err);
        }
    }

    if parsed.is_empty() {
        return;
    }

    println!();
    println!("Деревья разбора и последовательность действий:");

    for (idx, item) in parsed.iter().enumerate() {
        println!();
        println!(
            "{}. Строка {}: {} = {}",
            idx + 1,
            item.line,
            item.lhs,
            item.rhs
        );
        println!("Дерево:");
        println!("{}", render_tree(&item.tree));

        let mut steps = Vec::new();
        build_steps(&item.tree, &mut steps);

        println!("Шаги:");
        for (step_idx, step) in steps.iter().enumerate() {
            println!("{}. {}", step_idx + 1, step);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        eprintln!("Использование: {} <путь_к_файлу>", args[0]);
        process::exit(1);
    }

    let filename = &args[1];
    let content = match fs::read_to_string(filename) {
        Ok(value) => value,
        Err(err) => {
            eprintln!("Ошибка чтения '{}': {}", filename, err);
            process::exit(1);
        }
    };

    let (parsed, errors) = analyze_file(&content);
    print_analysis(&parsed, &errors, filename);

    if !errors.is_empty() {
        process::exit(1);
    }
}