mod lexer;
mod symbol_table;
mod token;

use std::collections::HashMap;
use std::env;
use std::fmt;
use std::fs;
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

#[derive(Debug, Clone)]
struct ParsedAssignment {
    line: usize,
    binding_name: String,
    params: Vec<String>,
    has_complex_params: bool,
    rhs_tree: Expr,
}

#[derive(Debug, Clone, PartialEq, Eq)]
enum SemanticType {
    Int,
    Integer,
    Nat,
    Double,
    String,
    Char,
    Bool,
    Unit,
    List(Box<SemanticType>),
    Maybe(Box<SemanticType>),
    Either(Box<SemanticType>, Box<SemanticType>),
    IO(Box<SemanticType>),
    Function(Vec<SemanticType>, Box<SemanticType>),
    TypeVar(String),
    Custom(String, Vec<SemanticType>),
    Unknown,
}

impl SemanticType {
    fn is_unknown(&self) -> bool {
        matches!(self, SemanticType::Unknown)
    }
}

impl fmt::Display for SemanticType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            SemanticType::Int => write!(f, "Int"),
            SemanticType::Integer => write!(f, "Integer"),
            SemanticType::Nat => write!(f, "Nat"),
            SemanticType::Double => write!(f, "Double"),
            SemanticType::String => write!(f, "String"),
            SemanticType::Char => write!(f, "Char"),
            SemanticType::Bool => write!(f, "Bool"),
            SemanticType::Unit => write!(f, "()"),
            SemanticType::List(inner) => write!(f, "List {}", inner),
            SemanticType::Maybe(inner) => write!(f, "Maybe {}", inner),
            SemanticType::Either(left, right) => write!(f, "Either {} {}", left, right),
            SemanticType::IO(inner) => write!(f, "IO {}", inner),
            SemanticType::Function(args, ret) => {
                if args.is_empty() {
                    write!(f, "{}", ret)
                } else {
                    let mut parts: Vec<String> = args.iter().map(ToString::to_string).collect();
                    parts.push(ret.to_string());
                    write!(f, "{}", parts.join(" -> "))
                }
            }
            SemanticType::TypeVar(name) => write!(f, "{}", name),
            SemanticType::Custom(name, args) => {
                if args.is_empty() {
                    write!(f, "{}", name)
                } else {
                    let rendered: Vec<String> = args.iter().map(ToString::to_string).collect();
                    write!(f, "{} {}", name, rendered.join(" "))
                }
            }
            SemanticType::Unknown => write!(f, "Unknown"),
        }
    }
}

#[derive(Debug, Clone)]
struct SignatureType {
    full: SemanticType,
    params: Vec<SemanticType>,
}

#[derive(Debug, Clone)]
struct TypeSignature {
    line: usize,
    parsed: SignatureType,
}

#[derive(Debug, Clone)]
struct SyntaxReport {
    assignments: Vec<ParsedAssignment>,
    signatures: HashMap<String, TypeSignature>,
    errors: Vec<String>,
}

#[derive(Debug, Clone)]
struct SymbolRecord {
    name: String,
    declared: Option<SemanticType>,
    inferred: Option<SemanticType>,
    declared_line: Option<usize>,
    defined_line: Option<usize>,
    arity: usize,
}

#[derive(Debug, Clone)]
struct AssignmentTypeInfo {
    line: usize,
    binding_name: String,
    inferred_rhs: SemanticType,
    expected_rhs: Option<SemanticType>,
}

#[derive(Debug, Clone)]
struct SemanticReport {
    errors: Vec<String>,
    warnings: Vec<String>,
    coercions: Vec<String>,
    symbols: Vec<SymbolRecord>,
    assignment_types: Vec<AssignmentTypeInfo>,
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

fn is_top_level_line(line: &str) -> bool {
    matches!(line.chars().next(), Some(ch) if !ch.is_whitespace())
}

fn lex_line_tokens(line: &str) -> Result<Vec<crate::token::Token>, ParseError> {
    let mut lexer = Lexer::new(line);
    let (tokens, errors) = lexer.tokenize();

    if let Some(first_error) = errors.first() {
        return Err(ParseError {
            column: first_error.column,
            message: first_error.message.clone(),
        });
    }

    Ok(tokens
        .into_iter()
        .filter(|token| !matches!(token.token_type, TokenType::Eof))
        .collect())
}

fn byte_index_from_column(line: &str, column: usize) -> Option<usize> {
    if column == 0 {
        return None;
    }

    let char_count = line.chars().count();
    if column == char_count + 1 {
        return Some(line.len());
    }

    line.char_indices().nth(column - 1).map(|(idx, _)| idx)
}

fn extract_top_level_signature(line: &str) -> Option<(String, String)> {
    if !is_top_level_line(line) {
        return None;
    }

    let tokens = lex_line_tokens(line).ok()?;
    if tokens.len() < 2 {
        return None;
    }

    match (&tokens[0].token_type, &tokens[1].token_type) {
        (TokenType::Identifier(name), TokenType::Colon) => {
            let colon_byte = byte_index_from_column(line, tokens[1].column)?;
            let raw_type = line[colon_byte + 1..].trim().to_string();
            Some((name.clone(), raw_type))
        }
        _ => None,
    }
}

fn extract_assignment_binding_and_params(lhs: &str) -> Option<(String, Vec<String>)> {
    let tokens = lex_line_tokens(lhs).ok()?;
    if tokens.is_empty() {
        return None;
    }

    let mut index = 0;
    if let TokenType::Keyword(keyword) = &tokens[0].token_type {
        if keyword == "let" {
            index = 1;
        }
    }

    let name = match tokens.get(index).map(|token| &token.token_type) {
        Some(TokenType::Identifier(value)) => value.clone(),
        _ => return None,
    };

    let mut params = Vec::new();
    for token in tokens.iter().skip(index + 1) {
        match &token.token_type {
            TokenType::Identifier(param) => params.push(param.clone()),
            _ => break,
        }
    }

    Some((name, params))
}

fn lhs_has_complex_patterns(lhs: &str, params_len: usize) -> bool {
    let tokens = match lex_line_tokens(lhs) {
        Ok(tokens) => tokens,
        Err(_) => return false,
    };

    if tokens.is_empty() {
        return false;
    }

    let mut index = 0usize;
    if let TokenType::Keyword(keyword) = &tokens[0].token_type {
        if keyword == "let" {
            index = 1;
        }
    }

    if !matches!(
        tokens.get(index).map(|token| &token.token_type),
        Some(TokenType::Identifier(_))
    ) {
        return false;
    }

    index + 1 + params_len < tokens.len()
}

fn extract_binding_head_without_assignment(line: &str) -> Option<(String, usize)> {
    let tokens = lex_line_tokens(line).ok()?;
    let first = tokens.first()?;

    match &first.token_type {
        TokenType::Identifier(name) => {
            let fallback_column = first.column + name.chars().count();
            let column = tokens
                .get(1)
                .map(|token| token.column)
                .unwrap_or(fallback_column);
            Some((name.clone(), column))
        }
        TokenType::Keyword(keyword) if keyword == "let" => {
            let second = tokens.get(1)?;
            if let TokenType::Identifier(name) = &second.token_type {
                let fallback_column = second.column + name.chars().count();
                let column = tokens
                    .get(2)
                    .map(|token| token.column)
                    .unwrap_or(fallback_column);
                Some((name.clone(), column))
            } else {
                None
            }
        }
        _ => None,
    }
}

fn split_top_level_arrows(input: &str) -> Vec<String> {
    let chars: Vec<(usize, char)> = input.char_indices().collect();
    let mut parts = Vec::new();

    let mut start = 0usize;
    let mut idx = 0usize;
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;
    let mut paren_depth = 0usize;
    let mut bracket_depth = 0usize;
    let mut brace_depth = 0usize;

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

        match ch {
            '(' => paren_depth += 1,
            ')' => paren_depth = paren_depth.saturating_sub(1),
            '[' => bracket_depth += 1,
            ']' => bracket_depth = bracket_depth.saturating_sub(1),
            '{' => brace_depth += 1,
            '}' => brace_depth = brace_depth.saturating_sub(1),
            _ => {}
        }

        if paren_depth == 0
            && bracket_depth == 0
            && brace_depth == 0
            && ch == '-'
            && idx + 1 < chars.len()
            && chars[idx + 1].1 == '>'
        {
            parts.push(input[start..byte_idx].trim().to_string());
            start = if idx + 2 < chars.len() {
                chars[idx + 2].0
            } else {
                input.len()
            };
            idx += 2;
            continue;
        }

        idx += 1;
    }

    parts.push(input[start..].trim().to_string());
    parts.into_iter().filter(|part| !part.is_empty()).collect()
}

fn split_top_level_whitespace(input: &str) -> Vec<String> {
    let chars: Vec<(usize, char)> = input.char_indices().collect();
    let mut parts = Vec::new();

    let mut start = None::<usize>;
    let mut idx = 0usize;
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;
    let mut paren_depth = 0usize;
    let mut bracket_depth = 0usize;
    let mut brace_depth = 0usize;

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
            if start.is_none() {
                start = Some(byte_idx);
            }
            idx += 1;
            continue;
        }

        if ch == '\'' {
            in_char = true;
            if start.is_none() {
                start = Some(byte_idx);
            }
            idx += 1;
            continue;
        }

        match ch {
            '(' => {
                paren_depth += 1;
                if start.is_none() {
                    start = Some(byte_idx);
                }
            }
            ')' => paren_depth = paren_depth.saturating_sub(1),
            '[' => {
                bracket_depth += 1;
                if start.is_none() {
                    start = Some(byte_idx);
                }
            }
            ']' => bracket_depth = bracket_depth.saturating_sub(1),
            '{' => {
                brace_depth += 1;
                if start.is_none() {
                    start = Some(byte_idx);
                }
            }
            '}' => brace_depth = brace_depth.saturating_sub(1),
            _ => {}
        }

        if paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 && ch.is_whitespace() {
            if let Some(start_byte) = start {
                parts.push(input[start_byte..byte_idx].trim().to_string());
                start = None;
            }
            idx += 1;
            continue;
        }

        if start.is_none() {
            start = Some(byte_idx);
        }

        idx += 1;
    }

    if let Some(start_byte) = start {
        parts.push(input[start_byte..].trim().to_string());
    }

    parts.into_iter().filter(|part| !part.is_empty()).collect()
}

fn strip_constraints(input: &str) -> String {
    let chars: Vec<(usize, char)> = input.char_indices().collect();

    let mut idx = 0usize;
    let mut last_cut = None::<usize>;
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;
    let mut paren_depth = 0usize;
    let mut bracket_depth = 0usize;
    let mut brace_depth = 0usize;

    while idx < chars.len() {
        let (_, ch) = chars[idx];

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

        match ch {
            '(' => paren_depth += 1,
            ')' => paren_depth = paren_depth.saturating_sub(1),
            '[' => bracket_depth += 1,
            ']' => bracket_depth = bracket_depth.saturating_sub(1),
            '{' => brace_depth += 1,
            '}' => brace_depth = brace_depth.saturating_sub(1),
            _ => {}
        }

        if paren_depth == 0
            && bracket_depth == 0
            && brace_depth == 0
            && ch == '='
            && idx + 1 < chars.len()
            && chars[idx + 1].1 == '>'
        {
            last_cut = Some(if idx + 2 < chars.len() {
                chars[idx + 2].0
            } else {
                input.len()
            });
            idx += 2;
            continue;
        }

        idx += 1;
    }

    if let Some(cut) = last_cut {
        input[cut..].trim().to_string()
    } else {
        input.trim().to_string()
    }
}

fn has_wrapping_parentheses(input: &str) -> bool {
    let chars: Vec<(usize, char)> = input.char_indices().collect();
    if chars.len() < 2
        || chars.first().map(|(_, c)| *c) != Some('(')
        || chars.last().map(|(_, c)| *c) != Some(')')
    {
        return false;
    }

    let mut depth = 0isize;
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;

    for (idx, (_, ch)) in chars.iter().enumerate() {
        if in_string {
            if escaped {
                escaped = false;
            } else if *ch == '\\' {
                escaped = true;
            } else if *ch == '"' {
                in_string = false;
            }
            continue;
        }

        if in_char {
            if escaped {
                escaped = false;
            } else if *ch == '\\' {
                escaped = true;
            } else if *ch == '\'' {
                in_char = false;
            }
            continue;
        }

        if *ch == '"' {
            in_string = true;
            continue;
        }

        if *ch == '\'' {
            in_char = true;
            continue;
        }

        if *ch == '(' {
            depth += 1;
        } else if *ch == ')' {
            depth -= 1;
            if depth == 0 && idx + 1 != chars.len() {
                return false;
            }
        }

        if depth < 0 {
            return false;
        }
    }

    depth == 0
}

fn strip_outer_parens_owned(input: &str) -> String {
    let mut current = input.trim().to_string();
    while has_wrapping_parentheses(&current) {
        current = current[1..current.len() - 1].trim().to_string();
    }
    current
}

fn find_top_level_colon(input: &str) -> Option<usize> {
    let chars: Vec<(usize, char)> = input.char_indices().collect();

    let mut idx = 0usize;
    let mut in_string = false;
    let mut in_char = false;
    let mut escaped = false;
    let mut paren_depth = 0usize;
    let mut bracket_depth = 0usize;
    let mut brace_depth = 0usize;

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

        match ch {
            '(' => paren_depth += 1,
            ')' => paren_depth = paren_depth.saturating_sub(1),
            '[' => bracket_depth += 1,
            ']' => bracket_depth = bracket_depth.saturating_sub(1),
            '{' => brace_depth += 1,
            '}' => brace_depth = brace_depth.saturating_sub(1),
            _ => {}
        }

        if paren_depth == 0 && bracket_depth == 0 && brace_depth == 0 && ch == ':' {
            let prev_is_colon = idx > 0 && chars[idx - 1].1 == ':';
            let next_is_colon = idx + 1 < chars.len() && chars[idx + 1].1 == ':';
            if !prev_is_colon && !next_is_colon {
                return Some(byte_idx);
            }
        }

        idx += 1;
    }

    None
}

fn primitive_type(name: &str) -> Option<SemanticType> {
    match name {
        "Int" => Some(SemanticType::Int),
        "Integer" => Some(SemanticType::Integer),
        "Nat" => Some(SemanticType::Nat),
        "Double" => Some(SemanticType::Double),
        "String" => Some(SemanticType::String),
        "Char" => Some(SemanticType::Char),
        "Bool" => Some(SemanticType::Bool),
        "()" => Some(SemanticType::Unit),
        _ => None,
    }
}

fn is_type_variable(name: &str) -> bool {
    matches!(name.chars().next(), Some(ch) if ch.is_ascii_lowercase())
}

fn build_type_application(head: &str, args: Vec<SemanticType>) -> SemanticType {
    match head {
        "List" => {
            let inner = args.first().cloned().unwrap_or(SemanticType::Unknown);
            SemanticType::List(Box::new(inner))
        }
        "Maybe" => {
            let inner = args.first().cloned().unwrap_or(SemanticType::Unknown);
            SemanticType::Maybe(Box::new(inner))
        }
        "Either" => {
            let left = args.first().cloned().unwrap_or(SemanticType::Unknown);
            let right = args.get(1).cloned().unwrap_or(SemanticType::Unknown);
            SemanticType::Either(Box::new(left), Box::new(right))
        }
        "IO" => {
            let inner = args.first().cloned().unwrap_or(SemanticType::Unknown);
            SemanticType::IO(Box::new(inner))
        }
        _ => {
            if let Some(primitive) = primitive_type(head) {
                if args.is_empty() {
                    primitive
                } else {
                    SemanticType::Custom(head.to_string(), args)
                }
            } else {
                SemanticType::Custom(head.to_string(), args)
            }
        }
    }
}

fn parse_type_atom(text: &str) -> SemanticType {
    let trimmed = text.trim();
    if trimmed.is_empty() {
        return SemanticType::Unknown;
    }

    if trimmed == "()" {
        return SemanticType::Unit;
    }

    if let Some(primitive) = primitive_type(trimmed) {
        return primitive;
    }

    if is_type_variable(trimmed) {
        return SemanticType::TypeVar(trimmed.to_string());
    }

    SemanticType::Custom(trimmed.to_string(), Vec::new())
}

fn parse_param_type_segment(segment: &str) -> SemanticType {
    let without_constraints = strip_constraints(segment);
    let stripped = strip_outer_parens_owned(&without_constraints);

    if let Some(colon_idx) = find_top_level_colon(&stripped) {
        let rhs = stripped[colon_idx + 1..].trim();
        return parse_type_expr(rhs);
    }

    parse_type_expr(&stripped)
}

fn parse_type_application(text: &str) -> SemanticType {
    let trimmed = text.trim();
    if trimmed.is_empty() {
        return SemanticType::Unknown;
    }

    if trimmed.starts_with('[') && trimmed.ends_with(']') && trimmed.len() >= 2 {
        let inner = &trimmed[1..trimmed.len() - 1];
        return SemanticType::List(Box::new(parse_type_expr(inner)));
    }

    let parts = split_top_level_whitespace(trimmed);
    if parts.is_empty() {
        return SemanticType::Unknown;
    }

    if parts.len() == 1 {
        return parse_type_atom(&parts[0]);
    }

    let head = &parts[0];
    let args: Vec<SemanticType> = parts[1..]
        .iter()
        .map(|part| parse_type_expr(part))
        .collect();
    build_type_application(head, args)
}

fn parse_type_expr(text: &str) -> SemanticType {
    let without_constraints = strip_constraints(text);
    let stripped = strip_outer_parens_owned(&without_constraints);

    if stripped.is_empty() {
        return SemanticType::Unknown;
    }

    let parts = split_top_level_arrows(&stripped);
    if parts.len() > 1 {
        let mut parsed_parts = Vec::new();
        for (idx, part) in parts.iter().enumerate() {
            if idx + 1 == parts.len() {
                parsed_parts.push(parse_type_expr(part));
            } else {
                parsed_parts.push(parse_param_type_segment(part));
            }
        }

        let result = parsed_parts.pop().unwrap_or(SemanticType::Unknown);
        return SemanticType::Function(parsed_parts, Box::new(result));
    }

    parse_type_application(&stripped)
}

fn parse_signature_type(raw: &str) -> SignatureType {
    let full = parse_type_expr(raw);
    match &full {
        SemanticType::Function(params, _) => SignatureType {
            full: full.clone(),
            params: params.clone(),
        },
        _ => SignatureType {
            full: full.clone(),
            params: Vec::new(),
        },
    }
}

fn builtin_identifier_type(name: &str) -> Option<SemanticType> {
    match name {
        "True" | "False" => Some(SemanticType::Bool),
        "Z" => Some(SemanticType::Nat),
        "Nothing" => Some(SemanticType::Maybe(Box::new(SemanticType::Unknown))),
        "Just" => Some(SemanticType::Function(
            vec![SemanticType::Unknown],
            Box::new(SemanticType::Maybe(Box::new(SemanticType::Unknown))),
        )),
        "Left" => Some(SemanticType::Function(
            vec![SemanticType::Unknown],
            Box::new(SemanticType::Either(
                Box::new(SemanticType::Unknown),
                Box::new(SemanticType::Unknown),
            )),
        )),
        "Right" => Some(SemanticType::Function(
            vec![SemanticType::Unknown],
            Box::new(SemanticType::Either(
                Box::new(SemanticType::Unknown),
                Box::new(SemanticType::Unknown),
            )),
        )),
        _ => None,
    }
}

fn infer_literal_type(literal: &str) -> SemanticType {
    if literal.starts_with('"') && literal.ends_with('"') {
        return SemanticType::String;
    }

    if literal.starts_with('\'') && literal.ends_with('\'') {
        return SemanticType::Char;
    }

    if literal.starts_with("0x") || literal.starts_with("0X") {
        return SemanticType::Int;
    }

    if literal.contains('.') || literal.contains('e') || literal.contains('E') {
        return SemanticType::Double;
    }

    SemanticType::Int
}

fn is_numeric_type(ty: &SemanticType) -> bool {
    matches!(
        ty,
        SemanticType::Int | SemanticType::Integer | SemanticType::Nat | SemanticType::Double
    )
}

fn numeric_rank(ty: &SemanticType) -> Option<u8> {
    match ty {
        SemanticType::Nat => Some(0),
        SemanticType::Int => Some(1),
        SemanticType::Integer => Some(2),
        SemanticType::Double => Some(3),
        _ => None,
    }
}

fn rank_to_type(rank: u8) -> SemanticType {
    match rank {
        0 => SemanticType::Nat,
        1 => SemanticType::Int,
        2 => SemanticType::Integer,
        _ => SemanticType::Double,
    }
}

fn assignable_with_coercion(
    from: &SemanticType,
    to: &SemanticType,
) -> Result<Option<String>, String> {
    if from == to || from.is_unknown() || to.is_unknown() {
        return Ok(None);
    }

    if matches!(from, SemanticType::TypeVar(_)) || matches!(to, SemanticType::TypeVar(_)) {
        return Ok(None);
    }

    match (from, to) {
        (SemanticType::Nat, SemanticType::Int) => {
            Ok(Some("неявное приведение Nat -> Int".to_string()))
        }
        (SemanticType::Int, SemanticType::Integer) => {
            Ok(Some("неявное приведение Int -> Integer".to_string()))
        }
        (SemanticType::Nat, SemanticType::Integer) => {
            Ok(Some("неявное приведение Nat -> Integer".to_string()))
        }
        (SemanticType::Int, SemanticType::Double) => {
            Ok(Some("неявное приведение Int -> Double".to_string()))
        }
        (SemanticType::Nat, SemanticType::Double) => {
            Ok(Some("неявное приведение Nat -> Double".to_string()))
        }
        (SemanticType::Integer, SemanticType::Double) => {
            Ok(Some("неявное приведение Integer -> Double".to_string()))
        }
        (SemanticType::List(from_inner), SemanticType::List(to_inner)) => {
            assignable_with_coercion(from_inner, to_inner)
        }
        (SemanticType::Maybe(from_inner), SemanticType::Maybe(to_inner)) => {
            assignable_with_coercion(from_inner, to_inner)
        }
        (SemanticType::Either(from_left, from_right), SemanticType::Either(to_left, to_right)) => {
            let mut notes = Vec::new();
            if let Some(note) = assignable_with_coercion(from_left, to_left)? {
                notes.push(note);
            }
            if let Some(note) = assignable_with_coercion(from_right, to_right)? {
                notes.push(note);
            }

            if notes.is_empty() {
                Ok(None)
            } else {
                Ok(Some(notes.join("; ")))
            }
        }
        (SemanticType::IO(from_inner), SemanticType::IO(to_inner)) => {
            assignable_with_coercion(from_inner, to_inner)
        }
        (SemanticType::List(from_inner), SemanticType::Custom(name, to_args))
            if name == "Vect" && !to_args.is_empty() =>
        {
            let target_elem = to_args.last().cloned().unwrap_or(SemanticType::Unknown);
            let note = assignable_with_coercion(from_inner, &target_elem)?;
            let base_note = format!("неявное приведение List -> {}", to);
            if let Some(extra) = note {
                Ok(Some(format!("{}; {}", base_note, extra)))
            } else {
                Ok(Some(base_note))
            }
        }
        (SemanticType::Function(from_args, from_ret), SemanticType::Function(to_args, to_ret)) => {
            if from_args.len() != to_args.len() {
                return Err(format!(
                    "ожидалась функция с {} параметрами, получена функция с {} параметрами",
                    to_args.len(),
                    from_args.len()
                ));
            }

            let mut notes = Vec::new();
            for (from_arg, to_arg) in from_args.iter().zip(to_args.iter()) {
                if let Some(note) = assignable_with_coercion(from_arg, to_arg)? {
                    notes.push(note);
                }
            }
            if let Some(note) = assignable_with_coercion(from_ret, to_ret)? {
                notes.push(note);
            }

            if notes.is_empty() {
                Ok(None)
            } else {
                Ok(Some(notes.join("; ")))
            }
        }
        (SemanticType::Custom(from_name, from_args), SemanticType::Custom(to_name, to_args))
            if from_name == to_name && from_args.len() == to_args.len() =>
        {
            let mut notes = Vec::new();
            for (from_arg, to_arg) in from_args.iter().zip(to_args.iter()) {
                if let Some(note) = assignable_with_coercion(from_arg, to_arg)? {
                    notes.push(note);
                }
            }
            if notes.is_empty() {
                Ok(None)
            } else {
                Ok(Some(notes.join("; ")))
            }
        }
        _ => Err(format!("ожидался тип {}, получен {}", to, from)),
    }
}

fn promote_numeric_types(
    left: &SemanticType,
    right: &SemanticType,
    line: usize,
    context: &str,
    errors: &mut Vec<String>,
    coercions: &mut Vec<String>,
) -> SemanticType {
    let Some(left_rank) = numeric_rank(left) else {
        if !left.is_unknown() && !right.is_unknown() {
            errors.push(format!(
                "Строка {}: в {} ожидается числовой тип слева, получен {}",
                line, context, left
            ));
        }
        return SemanticType::Unknown;
    };

    let Some(right_rank) = numeric_rank(right) else {
        if !left.is_unknown() && !right.is_unknown() {
            errors.push(format!(
                "Строка {}: в {} ожидается числовой тип справа, получен {}",
                line, context, right
            ));
        }
        return SemanticType::Unknown;
    };

    let promoted_rank = left_rank.max(right_rank);
    let promoted_type = rank_to_type(promoted_rank);

    if let Ok(Some(note)) = assignable_with_coercion(left, &promoted_type) {
        coercions.push(format!("Строка {}: {} ({})", line, note, context));
    }

    if let Ok(Some(note)) = assignable_with_coercion(right, &promoted_type) {
        coercions.push(format!("Строка {}: {} ({})", line, note, context));
    }

    promoted_type
}

fn apply_function_type(
    function_type: SemanticType,
    argument_type: SemanticType,
    line: usize,
    context: &str,
    errors: &mut Vec<String>,
    coercions: &mut Vec<String>,
) -> SemanticType {
    match function_type {
        SemanticType::Function(args, result) => {
            if args.is_empty() {
                errors.push(format!(
                    "Строка {}: в {} функция не принимает аргументы",
                    line, context
                ));
                return SemanticType::Unknown;
            }

            let expected = &args[0];
            match assignable_with_coercion(&argument_type, expected) {
                Ok(Some(note)) => {
                    coercions.push(format!("Строка {}: {} ({})", line, note, context));
                }
                Ok(None) => {}
                Err(msg) => {
                    if !argument_type.is_unknown() && !expected.is_unknown() {
                        errors.push(format!("Строка {}: в {} {}", line, context, msg));
                    }
                }
            }

            if args.len() == 1 {
                result.as_ref().clone()
            } else {
                SemanticType::Function(args[1..].to_vec(), result)
            }
        }
        SemanticType::Unknown => SemanticType::Unknown,
        other => {
            if !argument_type.is_unknown() {
                errors.push(format!(
                    "Строка {}: в {} пытаемся применить как функцию значение типа {}",
                    line, context, other
                ));
            }
            SemanticType::Unknown
        }
    }
}

fn infer_expr_type(
    expr: &Expr,
    global_env: &HashMap<String, SemanticType>,
    local_env: &HashMap<String, SemanticType>,
    line: usize,
    errors: &mut Vec<String>,
    coercions: &mut Vec<String>,
) -> SemanticType {
    match expr {
        Expr::Identifier(name) => {
            if let Some(ty) = local_env.get(name) {
                ty.clone()
            } else if let Some(ty) = global_env.get(name) {
                ty.clone()
            } else if let Some(ty) = builtin_identifier_type(name) {
                ty
            } else {
                SemanticType::Unknown
            }
        }
        Expr::Literal(value) => infer_literal_type(value),
        Expr::Unary { op, expr } => {
            let inner_type = infer_expr_type(expr, global_env, local_env, line, errors, coercions);
            if inner_type.is_unknown() {
                return SemanticType::Unknown;
            }

            match op.as_str() {
                "+" | "-" => {
                    if is_numeric_type(&inner_type) {
                        inner_type
                    } else {
                        errors.push(format!(
                            "Строка {}: унарный '{}' применён к нечисловому типу {}",
                            line, op, inner_type
                        ));
                        SemanticType::Unknown
                    }
                }
                "!" => {
                    if inner_type == SemanticType::Bool {
                        SemanticType::Bool
                    } else {
                        errors.push(format!(
                            "Строка {}: унарный '{}' ожидает Bool, получен {}",
                            line, op, inner_type
                        ));
                        SemanticType::Unknown
                    }
                }
                "~" => {
                    if matches!(
                        inner_type,
                        SemanticType::Int | SemanticType::Integer | SemanticType::Nat
                    ) {
                        SemanticType::Int
                    } else {
                        errors.push(format!(
                            "Строка {}: унарный '{}' ожидает целочисленный тип, получен {}",
                            line, op, inner_type
                        ));
                        SemanticType::Unknown
                    }
                }
                _ => SemanticType::Unknown,
            }
        }
        Expr::Binary { op, left, right } => {
            let left_type = infer_expr_type(left, global_env, local_env, line, errors, coercions);
            let right_type = infer_expr_type(right, global_env, local_env, line, errors, coercions);

            match op.as_str() {
                "+" | "-" | "*" | "/" | "^" => {
                    promote_numeric_types(&left_type, &right_type, line, op, errors, coercions)
                }
                "%" => {
                    let promoted =
                        promote_numeric_types(&left_type, &right_type, line, op, errors, coercions);
                    if promoted == SemanticType::Double {
                        errors.push(format!(
                            "Строка {}: оператор '%' не поддерживает Double",
                            line
                        ));
                        SemanticType::Unknown
                    } else {
                        promoted
                    }
                }
                "++" => {
                    if left_type == SemanticType::String && right_type == SemanticType::String {
                        return SemanticType::String;
                    }

                    match (&left_type, &right_type) {
                        (SemanticType::List(left_inner), SemanticType::List(right_inner)) => {
                            match assignable_with_coercion(left_inner, right_inner) {
                                Ok(Some(note)) => {
                                    coercions
                                        .push(format!("Строка {}: {} (оператор '++')", line, note));
                                    SemanticType::List(right_inner.clone())
                                }
                                Ok(None) => SemanticType::List(left_inner.clone()),
                                Err(_) => match assignable_with_coercion(right_inner, left_inner) {
                                    Ok(Some(note)) => {
                                        coercions.push(format!(
                                            "Строка {}: {} (оператор '++')",
                                            line, note
                                        ));
                                        SemanticType::List(left_inner.clone())
                                    }
                                    Ok(None) => SemanticType::List(left_inner.clone()),
                                    Err(msg) => {
                                        if !left_type.is_unknown() && !right_type.is_unknown() {
                                            errors.push(format!(
                                                "Строка {}: несовместимые типы для '++': {}",
                                                line, msg
                                            ));
                                        }
                                        SemanticType::Unknown
                                    }
                                },
                            }
                        }
                        _ => {
                            if !left_type.is_unknown() && !right_type.is_unknown() {
                                errors.push(format!(
                                    "Строка {}: оператор '++' поддерживает только String или List, получены {} и {}",
                                    line, left_type, right_type
                                ));
                            }
                            SemanticType::Unknown
                        }
                    }
                }
                "<" | ">" | "<=" | ">=" => {
                    if left_type.is_unknown() || right_type.is_unknown() {
                        return SemanticType::Unknown;
                    }

                    if is_numeric_type(&left_type) && is_numeric_type(&right_type) {
                        promote_numeric_types(&left_type, &right_type, line, op, errors, coercions);
                        return SemanticType::Bool;
                    }

                    match assignable_with_coercion(&left_type, &right_type)
                        .or_else(|_| assignable_with_coercion(&right_type, &left_type))
                    {
                        Ok(Some(note)) => {
                            coercions
                                .push(format!("Строка {}: {} (оператор '{}')", line, note, op));
                            SemanticType::Bool
                        }
                        Ok(None) => SemanticType::Bool,
                        Err(msg) => {
                            errors.push(format!(
                                "Строка {}: сравнение '{}' невозможно: {}",
                                line, op, msg
                            ));
                            SemanticType::Unknown
                        }
                    }
                }
                "==" | "/=" => {
                    if left_type.is_unknown() || right_type.is_unknown() {
                        return SemanticType::Unknown;
                    }

                    match assignable_with_coercion(&left_type, &right_type)
                        .or_else(|_| assignable_with_coercion(&right_type, &left_type))
                    {
                        Ok(Some(note)) => {
                            coercions
                                .push(format!("Строка {}: {} (оператор '{}')", line, note, op));
                            SemanticType::Bool
                        }
                        Ok(None) => SemanticType::Bool,
                        Err(msg) => {
                            errors.push(format!(
                                "Строка {}: сравнение '{}' невозможно: {}",
                                line, op, msg
                            ));
                            SemanticType::Unknown
                        }
                    }
                }
                "&&" | "||" => {
                    if left_type == SemanticType::Bool && right_type == SemanticType::Bool {
                        SemanticType::Bool
                    } else {
                        if !left_type.is_unknown() && !right_type.is_unknown() {
                            errors.push(format!(
                                "Строка {}: оператор '{}' ожидает Bool и Bool, получены {} и {}",
                                line, op, left_type, right_type
                            ));
                        }
                        SemanticType::Unknown
                    }
                }
                "::" => match right_type {
                    SemanticType::List(inner) => {
                        match assignable_with_coercion(&left_type, &inner) {
                            Ok(Some(note)) => {
                                coercions
                                    .push(format!("Строка {}: {} (оператор '::')", line, note));
                            }
                            Ok(None) => {}
                            Err(msg) => {
                                if !left_type.is_unknown() && !inner.is_unknown() {
                                    errors.push(format!(
                                        "Строка {}: несогласованные типы в '::': {}",
                                        line, msg
                                    ));
                                }
                            }
                        }
                        SemanticType::List(inner)
                    }
                    SemanticType::Unknown => SemanticType::List(Box::new(left_type)),
                    other => {
                        if !left_type.is_unknown() && !other.is_unknown() {
                            errors.push(format!(
                                "Строка {}: справа от '::' ожидается List, получен {}",
                                line, other
                            ));
                        }
                        SemanticType::Unknown
                    }
                },
                "$" => {
                    let context = format!("оператор '$' ('{}')", op);
                    apply_function_type(left_type, right_type, line, &context, errors, coercions)
                }
                _ => {
                    if let Some(op_type) = global_env.get(op) {
                        let after_first = apply_function_type(
                            op_type.clone(),
                            left_type,
                            line,
                            "пользовательский инфиксный оператор",
                            errors,
                            coercions,
                        );
                        apply_function_type(
                            after_first,
                            right_type,
                            line,
                            "пользовательский инфиксный оператор",
                            errors,
                            coercions,
                        )
                    } else {
                        SemanticType::Unknown
                    }
                }
            }
        }
        Expr::Apply { func, arg } => {
            let function_type =
                infer_expr_type(func, global_env, local_env, line, errors, coercions);
            let argument_type =
                infer_expr_type(arg, global_env, local_env, line, errors, coercions);

            apply_function_type(
                function_type,
                argument_type,
                line,
                "применение функции",
                errors,
                coercions,
            )
        }
        Expr::Lambda { params, body } => {
            let mut lambda_env = local_env.clone();
            let mut param_types = Vec::new();
            for param in params {
                lambda_env.insert(param.clone(), SemanticType::Unknown);
                param_types.push(SemanticType::Unknown);
            }

            let body_type = infer_expr_type(body, global_env, &lambda_env, line, errors, coercions);
            SemanticType::Function(param_types, Box::new(body_type))
        }
        Expr::List(items) => {
            if items.is_empty() {
                return SemanticType::List(Box::new(SemanticType::Unknown));
            }

            let mut iter = items.iter();
            let first = iter
                .next()
                .map(|item| infer_expr_type(item, global_env, local_env, line, errors, coercions))
                .unwrap_or(SemanticType::Unknown);

            let mut current = first;
            for item in iter {
                let ty = infer_expr_type(item, global_env, local_env, line, errors, coercions);
                if current.is_unknown() {
                    current = ty;
                    continue;
                }
                if ty.is_unknown() {
                    continue;
                }

                if let Ok(Some(note)) = assignable_with_coercion(&ty, &current) {
                    coercions.push(format!("Строка {}: {} (элемент списка)", line, note));
                    continue;
                }

                if let Ok(Some(note)) = assignable_with_coercion(&current, &ty) {
                    coercions.push(format!("Строка {}: {} (элемент списка)", line, note));
                    current = ty;
                    continue;
                }

                match (
                    assignable_with_coercion(&ty, &current),
                    assignable_with_coercion(&current, &ty),
                ) {
                    (Ok(None), _) => {}
                    (_, Ok(None)) => current = ty,
                    (Err(_), Err(_)) => {
                        errors.push(format!(
                            "Строка {}: элементы списка имеют несовместимые типы: {} и {}",
                            line, current, ty
                        ));
                        current = SemanticType::Unknown;
                    }
                    _ => {}
                }
            }

            SemanticType::List(Box::new(current))
        }
        Expr::OperatorRef(op) => {
            if op == "&&" || op == "||" {
                SemanticType::Function(
                    vec![SemanticType::Bool, SemanticType::Bool],
                    Box::new(SemanticType::Bool),
                )
            } else {
                SemanticType::Function(
                    vec![SemanticType::Unknown, SemanticType::Unknown],
                    Box::new(SemanticType::Unknown),
                )
            }
        }
        Expr::SectionRight { op, rhs } => {
            let rhs_type = infer_expr_type(rhs, global_env, local_env, line, errors, coercions);
            if op == "&&" || op == "||" {
                match assignable_with_coercion(&rhs_type, &SemanticType::Bool) {
                    Ok(Some(note)) => {
                        coercions.push(format!(
                            "Строка {}: {} (частичное применение '{}')",
                            line, note, op
                        ));
                    }
                    Ok(None) => {}
                    Err(msg) => {
                        if !rhs_type.is_unknown() {
                            errors.push(format!(
                                "Строка {}: в частичном применении '{}' {}",
                                line, op, msg
                            ));
                        }
                    }
                }
                SemanticType::Function(vec![SemanticType::Bool], Box::new(SemanticType::Bool))
            } else {
                SemanticType::Function(vec![SemanticType::Unknown], Box::new(rhs_type))
            }
        }
    }
}

fn analyze_file(content: &str) -> SyntaxReport {
    let mut parsed = Vec::new();
    let mut errors = Vec::new();
    let mut signatures: HashMap<String, TypeSignature> = HashMap::new();
    let mut pending_signatures: HashMap<String, usize> = HashMap::new();

    for (idx, raw_line) in content.lines().enumerate() {
        let line_number = idx + 1;
        let clean_line = strip_line_comment(raw_line);
        let line = clean_line.trim_end();

        if line.trim().is_empty() {
            continue;
        }

        if let Some((name, raw_type)) = extract_top_level_signature(line) {
            if signatures.contains_key(&name) {
                errors.push(format!(
                    "Строка {}: повторное объявление сигнатуры для '{}'",
                    line_number, name
                ));
                continue;
            }

            let parsed_signature = parse_signature_type(&raw_type);
            signatures.insert(
                name.clone(),
                TypeSignature {
                    line: line_number,
                    parsed: parsed_signature,
                },
            );
            pending_signatures.insert(name, line_number);
            continue;
        }

        if !is_top_level_line(line) {
            continue;
        }

        let Some((lhs, rhs, rhs_col)) = find_assignment(line) else {
            match lex_line_tokens(line) {
                Ok(_) => {
                    if let Some((name, column)) = extract_binding_head_without_assignment(line) {
                        if let Some(signature_line) = pending_signatures.remove(&name) {
                            errors.push(format!(
                                "Строка {}: синтаксическая ошибка в колонке {}: ожидался '=' после '{}'. Сигнатура типа объявлена в строке {}",
                                line_number, column, name, signature_line
                            ));
                        }
                    }
                }
                Err(err) => {
                    errors.push(format!(
                        "Строка {}: лексическая ошибка в колонке {}: {}",
                        line_number, err.column, err.message
                    ));
                }
            }
            continue;
        };

        if lhs.trim().is_empty() {
            continue;
        }

        let lhs_trimmed_start = lhs.trim_start();
        if lhs_trimmed_start.starts_with("data ")
            || lhs_trimmed_start.starts_with("record ")
            || lhs_trimmed_start.starts_with("interface ")
            || lhs_trimmed_start.starts_with("implementation ")
        {
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

        let (binding_name, params) = extract_assignment_binding_and_params(&lhs)
            .unwrap_or_else(|| (lhs.clone(), Vec::new()));
        let has_complex_params = lhs_has_complex_patterns(&lhs, params.len());

        pending_signatures.remove(&binding_name);

        parsed.push(ParsedAssignment {
            line: line_number,
            binding_name,
            params,
            has_complex_params,
            rhs_tree,
        });
    }

    for (name, signature_line) in pending_signatures {
        errors.push(format!(
            "Строка {}: для '{}' объявлена сигнатура типа, но определение с '=' не найдено",
            signature_line, name
        ));
    }

    SyntaxReport {
        assignments: parsed,
        signatures,
        errors,
    }
}

fn analyze_semantics(
    assignments: &[ParsedAssignment],
    signatures: &HashMap<String, TypeSignature>,
) -> SemanticReport {
    let mut errors = Vec::new();
    let mut warnings = Vec::new();
    let mut coercions = Vec::new();
    let mut assignment_types = Vec::new();

    let mut symbol_map: HashMap<String, SymbolRecord> = HashMap::new();
    let mut global_types: HashMap<String, SemanticType> = HashMap::new();

    for (name, signature) in signatures {
        global_types.insert(name.clone(), signature.parsed.full.clone());
        symbol_map.insert(
            name.clone(),
            SymbolRecord {
                name: name.clone(),
                declared: Some(signature.parsed.full.clone()),
                inferred: None,
                declared_line: Some(signature.line),
                defined_line: None,
                arity: signature.parsed.params.len(),
            },
        );
    }

    for assignment in assignments {
        let mut local_types = HashMap::new();
        let mut param_types = vec![SemanticType::Unknown; assignment.params.len()];
        let mut expected_rhs = None::<SemanticType>;
        let mut resolved_arity = assignment.params.len();

        if let Some(signature) = signatures.get(&assignment.binding_name) {
            match &signature.parsed.full {
                SemanticType::Function(args, result) => {
                    resolved_arity = args.len();

                    if args.len() < assignment.params.len() {
                        errors.push(format!(
                            "Строка {}: в определении '{}' параметров ({}) больше, чем в сигнатуре ({})",
                            assignment.line,
                            assignment.binding_name,
                            assignment.params.len(),
                            args.len()
                        ));
                    }

                    for (idx, param_name) in assignment.params.iter().enumerate() {
                        let ty = args.get(idx).cloned().unwrap_or(SemanticType::Unknown);
                        param_types[idx] = ty.clone();
                        local_types.insert(param_name.clone(), ty);
                    }

                    let consumed_params = if assignment.has_complex_params {
                        args.len()
                    } else {
                        assignment.params.len()
                    };

                    if args.len() >= consumed_params {
                        let remaining = args[consumed_params..].to_vec();
                        expected_rhs = if remaining.is_empty() {
                            Some(result.as_ref().clone())
                        } else {
                            Some(SemanticType::Function(remaining, result.clone()))
                        };
                    } else {
                        expected_rhs = Some(result.as_ref().clone());
                    }
                }
                declared_non_function => {
                    if !assignment.params.is_empty() {
                        errors.push(format!(
                            "Строка {}: '{}' объявлен как нефункциональный тип {}, но определён с параметрами",
                            assignment.line, assignment.binding_name, declared_non_function
                        ));
                    }
                    expected_rhs = Some(declared_non_function.clone());
                }
            }
        } else {
            resolved_arity = assignment.params.len();
            for param_name in &assignment.params {
                local_types.insert(param_name.clone(), SemanticType::Unknown);
            }
        }

        let rhs_type = infer_expr_type(
            &assignment.rhs_tree,
            &global_types,
            &local_types,
            assignment.line,
            &mut errors,
            &mut coercions,
        );

        if let Some(expected) = expected_rhs.clone() {
            match assignable_with_coercion(&rhs_type, &expected) {
                Ok(Some(note)) => {
                    coercions.push(format!(
                        "Строка {}: {} (присваивание '{}')",
                        assignment.line, note, assignment.binding_name
                    ));
                }
                Ok(None) => {}
                Err(msg) => {
                    if !rhs_type.is_unknown() && !expected.is_unknown() {
                        errors.push(format!(
                            "Строка {}: в '{}' {}",
                            assignment.line, assignment.binding_name, msg
                        ));
                    }
                }
            }
        }

        let inferred_binding_type = if assignment.params.is_empty() {
            if assignment.has_complex_params {
                signatures
                    .get(&assignment.binding_name)
                    .map(|signature| signature.parsed.full.clone())
                    .unwrap_or_else(|| rhs_type.clone())
            } else {
                rhs_type.clone()
            }
        } else {
            SemanticType::Function(param_types.clone(), Box::new(rhs_type.clone()))
        };

        let visible_type = signatures
            .get(&assignment.binding_name)
            .map(|sig| sig.parsed.full.clone())
            .unwrap_or_else(|| inferred_binding_type.clone());

        global_types.insert(assignment.binding_name.clone(), visible_type);

        symbol_map
            .entry(assignment.binding_name.clone())
            .and_modify(|entry| {
                entry.inferred = Some(inferred_binding_type.clone());
                entry.defined_line = Some(assignment.line);
                entry.arity = resolved_arity;
            })
            .or_insert(SymbolRecord {
                name: assignment.binding_name.clone(),
                declared: None,
                inferred: Some(inferred_binding_type.clone()),
                declared_line: None,
                defined_line: Some(assignment.line),
                arity: resolved_arity,
            });

        assignment_types.push(AssignmentTypeInfo {
            line: assignment.line,
            binding_name: assignment.binding_name.clone(),
            inferred_rhs: rhs_type,
            expected_rhs,
        });

        if !signatures.contains_key(&assignment.binding_name)
            && matches!(
                inferred_binding_type,
                SemanticType::Function(ref args, _) if args.iter().all(SemanticType::is_unknown)
            )
        {
            warnings.push(format!(
                "Строка {}: для '{}' тип параметров не определён (нет сигнатуры)",
                assignment.line, assignment.binding_name
            ));
        }
    }

    let mut symbols: Vec<SymbolRecord> = symbol_map.into_values().collect();
    symbols.sort_by(|a, b| a.name.cmp(&b.name));

    assignment_types.sort_by_key(|entry| entry.line);

    SemanticReport {
        errors,
        warnings,
        coercions,
        symbols,
        assignment_types,
    }
}

fn render_optional_type(value: &Option<SemanticType>) -> String {
    value
        .as_ref()
        .map(ToString::to_string)
        .unwrap_or_else(|| "-".to_string())
}

fn render_optional_line(value: Option<usize>) -> String {
    value
        .map(|line| line.to_string())
        .unwrap_or_else(|| "-".to_string())
}

fn print_report(file: &str, syntax: &SyntaxReport, semantic: &SemanticReport) {
    println!("Файл: {}", file);
    println!("Разобрано определений: {}", syntax.assignments.len());
    println!("Обнаружено сигнатур типов: {}", syntax.signatures.len());
    println!("Синтаксических ошибок: {}", syntax.errors.len());
    println!("Семантических ошибок: {}", semantic.errors.len());

    if !syntax.errors.is_empty() {
        println!();
        println!("Синтаксические ошибки:");
        for error in &syntax.errors {
            println!("- {}", error);
        }
    }

    if !semantic.errors.is_empty() {
        println!();
        println!("Семантические ошибки:");
        for error in &semantic.errors {
            println!("- {}", error);
        }
    }

    if !semantic.warnings.is_empty() {
        println!();
        println!("Предупреждения:");
        for warning in &semantic.warnings {
            println!("- {}", warning);
        }
    }

    if !semantic.coercions.is_empty() {
        println!();
        println!("Неявные приведения типов:");
        for item in &semantic.coercions {
            println!("- {}", item);
        }
    }

    if !semantic.assignment_types.is_empty() {
        println!();
        println!("Типы выражений в определениях:");
        for item in &semantic.assignment_types {
            if let Some(expected) = &item.expected_rhs {
                println!(
                    "- Строка {}: {} = ... | выражение: {} | ожидается: {}",
                    item.line, item.binding_name, item.inferred_rhs, expected
                );
            } else {
                println!(
                    "- Строка {}: {} = ... | выражение: {}",
                    item.line, item.binding_name, item.inferred_rhs
                );
            }
        }
    }

    if !semantic.symbols.is_empty() {
        println!();
        println!("Таблица символов:");
        println!(
            "{:<24} | {:<24} | {:<24} | {:<7} | {:<8} | {:<8}",
            "Имя", "Объявленный тип", "Выведенный тип", "Арность", "Сигнатура", "Определ."
        );
        println!(
            "{}",
            "-".repeat(24 + 3 + 24 + 3 + 24 + 3 + 7 + 3 + 8 + 3 + 8)
        );

        for symbol in &semantic.symbols {
            println!(
                "{:<24} | {:<24} | {:<24} | {:<7} | {:<8} | {:<8}",
                symbol.name,
                render_optional_type(&symbol.declared),
                render_optional_type(&symbol.inferred),
                symbol.arity,
                render_optional_line(symbol.declared_line),
                render_optional_line(symbol.defined_line),
            );
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

    let syntax_report = analyze_file(&content);
    let semantic_report = analyze_semantics(&syntax_report.assignments, &syntax_report.signatures);

    print_report(filename, &syntax_report, &semantic_report);

    if !syntax_report.errors.is_empty() || !semantic_report.errors.is_empty() {
        process::exit(1);
    }
}
