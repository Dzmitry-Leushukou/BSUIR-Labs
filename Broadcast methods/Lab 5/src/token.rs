#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    Keyword(String),

    Identifier(String),
    IntLiteral(String),
    HexLiteral(String),
    FloatLiteral(String),
    StringLiteral(String),
    CharLiteral(String),

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Caret,
    Equal,
    EqualEqual,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    AmpAmp,
    PipePipe,
    Bang,
    Colon,
    ColonColon,
    Arrow,
    FatArrow,
    Backslash,
    Pipe,
    At,
    Question,
    DotDot,
    DotDotDot,
    DollarSign,
    Tilde,
    ColonGreater,
    LessColon,

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

    Eof,
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct Token {
    pub token_type: TokenType,
    pub line: usize,
    pub column: usize,
    pub table_index: Option<usize>,
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct LexicalError {
    pub line: usize,
    pub column: usize,
    pub message: String,
}
