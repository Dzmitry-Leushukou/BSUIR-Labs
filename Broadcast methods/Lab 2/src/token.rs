/// Token types for Idris 2 lexical analysis
#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    // Keywords
    Keyword(String),

    // Identifiers and Literals
    Identifier(String),
    IntLiteral(String),
    HexLiteral(String),
    FloatLiteral(String),
    StringLiteral(String),
    CharLiteral(String),

    // Operators and Delimiters
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

    // Delimiters
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

    // End of file
    Eof,
}

/// Represents a single token
#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub line: usize,
    pub column: usize,
    pub table_index: Option<usize>, // For identifiers and constants
}

/// Represents a lexical error
#[derive(Debug, Clone)]
pub struct LexicalError {
    pub line: usize,
    pub column: usize,
    pub message: String,
}
