use crate::{Expr, ParsedAssignment};
use std::collections::{HashMap, HashSet};
use std::fmt;

#[derive(Debug, Clone)]
pub enum RuntimeValue {
    Int(i64),
    Double(f64),
    Bool(bool),
    String(String),
    Char(char),
    List(Vec<RuntimeValue>),
    Tagged { name: String, fields: Vec<RuntimeValue> },
    Function(Box<RuntimeFunction>),
}

impl fmt::Display for RuntimeValue {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RuntimeValue::Int(value) => write!(f, "{}", value),
            RuntimeValue::Double(value) => write!(f, "{}", value),
            RuntimeValue::Bool(value) => {
                if *value {
                    write!(f, "True")
                } else {
                    write!(f, "False")
                }
            }
            RuntimeValue::String(value) => write!(f, "\"{}\"", escape_string(value)),
            RuntimeValue::Char(value) => write!(f, "'{}'", escape_char(&value.to_string())),
            RuntimeValue::List(items) => {
                let rendered: Vec<String> = items.iter().map(ToString::to_string).collect();
                write!(f, "[{}]", rendered.join(", "))
            }
            RuntimeValue::Tagged { name, fields } => {
                if fields.is_empty() {
                    write!(f, "{}", name)
                } else {
                    let rendered: Vec<String> = fields.iter().map(ToString::to_string).collect();
                    write!(f, "{} {}", name, rendered.join(" "))
                }
            }
            RuntimeValue::Function(_) => write!(f, "<function>"),
        }
    }
}

#[derive(Debug, Clone)]
pub(crate) enum RuntimeFunction {
    User {
        function_id: usize,
        captured: HashMap<String, RuntimeValue>,
        applied: Vec<RuntimeValue>,
    },
    Builtin {
        kind: BuiltinKind,
        applied: Vec<RuntimeValue>,
    },
}

#[derive(Debug, Clone)]
pub(crate) enum BuiltinKind {
    OperatorRef(String),
    SectionRight { op: String, rhs: Box<RuntimeValue> },
    Constructor { name: String, arity: usize },
    Show,
    Mod,
}

#[derive(Debug, Clone)]
enum Instruction {
    PushLiteral(RuntimeValue),
    LoadName(String),
    BuildList(usize),
    Unary(String),
    Binary(String),
    MakeFunction(usize),
    MakeOperatorRef(String),
    MakeSectionRight(String),
    Call,
    Return,
}

impl fmt::Display for Instruction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Instruction::PushLiteral(value) => write!(f, "push {}", value),
            Instruction::LoadName(name) => write!(f, "load {}", name),
            Instruction::BuildList(len) => write!(f, "list {}", len),
            Instruction::Unary(op) => write!(f, "unary {}", op),
            Instruction::Binary(op) => write!(f, "binary {}", op),
            Instruction::MakeFunction(id) => write!(f, "lambda #{}", id),
            Instruction::MakeOperatorRef(op) => write!(f, "opref {}", op),
            Instruction::MakeSectionRight(op) => write!(f, "section_right {}", op),
            Instruction::Call => write!(f, "call"),
            Instruction::Return => write!(f, "ret"),
        }
    }
}

#[derive(Debug, Clone)]
struct BytecodeFunction {
    name: String,
    params: Vec<String>,
    code: Vec<Instruction>,
}

#[derive(Debug, Clone)]
struct GlobalDefinition {
    function_id: usize,
    arity: usize,
    line: usize,
}

#[derive(Debug, Clone)]
pub struct BytecodeProgram {
    functions: Vec<BytecodeFunction>,
    globals: HashMap<String, GlobalDefinition>,
    warnings: Vec<String>,
    errors: Vec<String>,
}

impl BytecodeProgram {
    pub fn has_errors(&self) -> bool {
        !self.errors.is_empty()
    }

    fn has_global(&self, name: &str) -> bool {
        self.globals.contains_key(name)
    }
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

fn unescape_literal_body(input: &str) -> Result<String, String> {
    let mut result = String::new();
    let mut chars = input.chars();

    while let Some(ch) = chars.next() {
        if ch != '\\' {
            result.push(ch);
            continue;
        }

        let Some(escaped) = chars.next() else {
            return Err("некорректная escape-последовательность в конце литерала".to_string());
        };

        match escaped {
            'n' => result.push('\n'),
            't' => result.push('\t'),
            'r' => result.push('\r'),
            '\\' => result.push('\\'),
            '"' => result.push('"'),
            '\'' => result.push('\''),
            other => result.push(other),
        }
    }

    Ok(result)
}

fn parse_runtime_literal(literal: &str) -> Result<RuntimeValue, String> {
    if literal.starts_with('"') && literal.ends_with('"') && literal.len() >= 2 {
        let raw = &literal[1..literal.len() - 1];
        let value = unescape_literal_body(raw)?;
        return Ok(RuntimeValue::String(value));
    }

    if literal.starts_with('\'') && literal.ends_with('\'') && literal.len() >= 3 {
        let raw = &literal[1..literal.len() - 1];
        let value = unescape_literal_body(raw)?;
        let mut iter = value.chars();
        let Some(ch) = iter.next() else {
            return Err("пустой символьный литерал".to_string());
        };
        if iter.next().is_some() {
            return Err(format!(
                "символьный литерал '{}' содержит более одного символа",
                literal
            ));
        }
        return Ok(RuntimeValue::Char(ch));
    }

    if literal.starts_with("0x") || literal.starts_with("0X") {
        let hex_part = &literal[2..];
        let parsed = i64::from_str_radix(hex_part, 16)
            .map_err(|_| format!("не удалось разобрать hex-литерал '{}'", literal))?;
        return Ok(RuntimeValue::Int(parsed));
    }

    if literal.contains('.') || literal.contains('e') || literal.contains('E') {
        let parsed = literal
            .parse::<f64>()
            .map_err(|_| format!("не удалось разобрать число '{}'", literal))?;
        return Ok(RuntimeValue::Double(parsed));
    }

    let parsed = literal
        .parse::<i64>()
        .map_err(|_| format!("не удалось разобрать целое число '{}'", literal))?;
    Ok(RuntimeValue::Int(parsed))
}

fn builtin_constant(name: &str) -> Option<RuntimeValue> {
    match name {
        "True" => Some(RuntimeValue::Bool(true)),
        "False" => Some(RuntimeValue::Bool(false)),
        "Nothing" => Some(RuntimeValue::Tagged {
            name: "Nothing".to_string(),
            fields: Vec::new(),
        }),
        "Z" => Some(RuntimeValue::Tagged {
            name: "Z".to_string(),
            fields: Vec::new(),
        }),
        _ => None,
    }
}

fn builtin_function(name: &str) -> Option<RuntimeValue> {
    let builtin = match name {
        "show" => BuiltinKind::Show,
        "mod" => BuiltinKind::Mod,
        "Just" | "Left" | "Right" | "S" => BuiltinKind::Constructor {
            name: name.to_string(),
            arity: 1,
        },
        _ => return None,
    };

    Some(RuntimeValue::Function(Box::new(RuntimeFunction::Builtin {
        kind: builtin,
        applied: Vec::new(),
    })))
}

fn builtin_arity(kind: &BuiltinKind) -> usize {
    match kind {
        BuiltinKind::OperatorRef(_) => 2,
        BuiltinKind::SectionRight { .. } => 1,
        BuiltinKind::Constructor { arity, .. } => *arity,
        BuiltinKind::Show => 1,
        BuiltinKind::Mod => 2,
    }
}

fn runtime_values_equal(left: &RuntimeValue, right: &RuntimeValue) -> Result<bool, String> {
    match (left, right) {
        (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(a == b),
        (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(a == b),
        (RuntimeValue::Int(a), RuntimeValue::Double(b)) => Ok((*a as f64) == *b),
        (RuntimeValue::Double(a), RuntimeValue::Int(b)) => Ok(*a == (*b as f64)),
        (RuntimeValue::Bool(a), RuntimeValue::Bool(b)) => Ok(a == b),
        (RuntimeValue::String(a), RuntimeValue::String(b)) => Ok(a == b),
        (RuntimeValue::Char(a), RuntimeValue::Char(b)) => Ok(a == b),
        (RuntimeValue::List(a), RuntimeValue::List(b)) => {
            if a.len() != b.len() {
                return Ok(false);
            }
            for (left_item, right_item) in a.iter().zip(b.iter()) {
                if !runtime_values_equal(left_item, right_item)? {
                    return Ok(false);
                }
            }
            Ok(true)
        }
        (
            RuntimeValue::Tagged {
                name: left_name,
                fields: left_fields,
            },
            RuntimeValue::Tagged {
                name: right_name,
                fields: right_fields,
            },
        ) => {
            if left_name != right_name || left_fields.len() != right_fields.len() {
                return Ok(false);
            }
            for (left_item, right_item) in left_fields.iter().zip(right_fields.iter()) {
                if !runtime_values_equal(left_item, right_item)? {
                    return Ok(false);
                }
            }
            Ok(true)
        }
        (RuntimeValue::Function(_), _) | (_, RuntimeValue::Function(_)) => {
            Err("сравнение функций не поддерживается".to_string())
        }
        _ => Ok(false),
    }
}

struct BytecodeCompiler {
    functions: Vec<BytecodeFunction>,
}

impl BytecodeCompiler {
    fn new() -> Self {
        Self {
            functions: Vec::new(),
        }
    }

    fn compile_program(mut self, assignments: &[ParsedAssignment]) -> BytecodeProgram {
        let mut globals: HashMap<String, GlobalDefinition> = HashMap::new();
        let mut warnings = Vec::new();
        let mut errors = Vec::new();

        for assignment in assignments {
            if assignment.has_complex_params {
                warnings.push(format!(
                    "Строка {}: '{}' использует образцы в параметрах; интерпретатор поддерживает только параметры-идентификаторы",
                    assignment.line, assignment.binding_name
                ));
                continue;
            }

            if globals.contains_key(&assignment.binding_name) {
                errors.push(format!(
                    "Строка {}: повторное определение '{}' не поддерживается интерпретатором",
                    assignment.line, assignment.binding_name
                ));
                continue;
            }

            let function_name = format!("{}@line{}", assignment.binding_name, assignment.line);
            let function_id = match self.compile_function(
                function_name,
                assignment.params.clone(),
                &assignment.rhs_tree,
            ) {
                Ok(id) => id,
                Err(err) => {
                    errors.push(format!(
                        "Строка {}: не удалось скомпилировать '{}' в байт-код: {}",
                        assignment.line, assignment.binding_name, err
                    ));
                    continue;
                }
            };

            globals.insert(
                assignment.binding_name.clone(),
                GlobalDefinition {
                    function_id,
                    arity: assignment.params.len(),
                    line: assignment.line,
                },
            );
        }

        BytecodeProgram {
            functions: self.functions,
            globals,
            warnings,
            errors,
        }
    }

    fn compile_function(
        &mut self,
        name: String,
        params: Vec<String>,
        body: &Expr,
    ) -> Result<usize, String> {
        let mut code = Vec::new();
        self.compile_expr(body, &mut code)?;
        code.push(Instruction::Return);

        let function_id = self.functions.len();
        self.functions.push(BytecodeFunction { name, params, code });
        Ok(function_id)
    }

    fn compile_expr(&mut self, expr: &Expr, code: &mut Vec<Instruction>) -> Result<(), String> {
        match expr {
            Expr::Identifier(name) => {
                code.push(Instruction::LoadName(name.clone()));
                Ok(())
            }
            Expr::Literal(value) => {
                let runtime_value = parse_runtime_literal(value)?;
                code.push(Instruction::PushLiteral(runtime_value));
                Ok(())
            }
            Expr::Unary { op, expr } => {
                self.compile_expr(expr, code)?;
                code.push(Instruction::Unary(op.clone()));
                Ok(())
            }
            Expr::Binary { op, left, right } => {
                self.compile_expr(left, code)?;
                self.compile_expr(right, code)?;
                code.push(Instruction::Binary(op.clone()));
                Ok(())
            }
            Expr::Apply { func, arg } => {
                self.compile_expr(func, code)?;
                self.compile_expr(arg, code)?;
                code.push(Instruction::Call);
                Ok(())
            }
            Expr::Lambda { params, body } => {
                let lambda_name = format!("lambda#{}", self.functions.len());
                let lambda_id = self.compile_function(lambda_name, params.clone(), body)?;
                code.push(Instruction::MakeFunction(lambda_id));
                Ok(())
            }
            Expr::List(items) => {
                for item in items {
                    self.compile_expr(item, code)?;
                }
                code.push(Instruction::BuildList(items.len()));
                Ok(())
            }
            Expr::OperatorRef(op) => {
                code.push(Instruction::MakeOperatorRef(op.clone()));
                Ok(())
            }
            Expr::SectionRight { op, rhs } => {
                self.compile_expr(rhs, code)?;
                code.push(Instruction::MakeSectionRight(op.clone()));
                Ok(())
            }
        }
    }
}

pub fn compile_to_bytecode(assignments: &[ParsedAssignment]) -> BytecodeProgram {
    BytecodeCompiler::new().compile_program(assignments)
}

pub struct Interpreter {
    program: BytecodeProgram,
    global_cache: HashMap<String, RuntimeValue>,
    evaluating: HashSet<String>,
}

impl Interpreter {
    pub fn new(program: BytecodeProgram) -> Self {
        Self {
            program,
            global_cache: HashMap::new(),
            evaluating: HashSet::new(),
        }
    }

    pub fn has_global(&self, name: &str) -> bool {
        self.program.has_global(name)
    }

    fn evaluate_global_constant(&mut self, name: &str) -> Result<RuntimeValue, String> {
        if let Some(value) = self.global_cache.get(name) {
            return Ok(value.clone());
        }

        let Some(definition) = self.program.globals.get(name).cloned() else {
            return Err(format!("идентификатор '{}' не определён", name));
        };

        if definition.arity != 0 {
            return Err(format!("'{}' имеет арность {}, а не 0", name, definition.arity));
        }

        if !self.evaluating.insert(name.to_string()) {
            return Err(format!(
                "обнаружена циклическая зависимость при вычислении '{}'",
                name
            ));
        }

        let result = self.execute_user_function(definition.function_id, HashMap::new(), Vec::new());
        self.evaluating.remove(name);

        if let Ok(value) = &result {
            self.global_cache.insert(name.to_string(), value.clone());
        }

        result
    }

    fn resolve_name(
        &mut self,
        name: &str,
        locals: &HashMap<String, RuntimeValue>,
    ) -> Result<RuntimeValue, String> {
        if let Some(value) = locals.get(name) {
            return Ok(value.clone());
        }

        if let Some(value) = builtin_constant(name) {
            return Ok(value);
        }

        if let Some(function) = builtin_function(name) {
            return Ok(function);
        }

        if let Some(definition) = self.program.globals.get(name).cloned() {
            if definition.arity == 0 {
                return self.evaluate_global_constant(name);
            }

            return Ok(RuntimeValue::Function(Box::new(RuntimeFunction::User {
                function_id: definition.function_id,
                captured: HashMap::new(),
                applied: Vec::new(),
            })));
        }

        Err(format!("неизвестный идентификатор '{}'", name))
    }

    fn execute_user_function(
        &mut self,
        function_id: usize,
        captured: HashMap<String, RuntimeValue>,
        args: Vec<RuntimeValue>,
    ) -> Result<RuntimeValue, String> {
        let Some(function) = self.program.functions.get(function_id).cloned() else {
            return Err(format!("байт-код функции #{} не найден", function_id));
        };

        if function.params.len() != args.len() {
            return Err(format!(
                "функция '{}' ожидала {} аргументов, получено {}",
                function.name,
                function.params.len(),
                args.len()
            ));
        }

        let mut locals = captured;
        for (param, arg) in function.params.iter().zip(args.into_iter()) {
            locals.insert(param.clone(), arg);
        }

        self.execute_bytecode(function_id, locals)
    }

    fn execute_bytecode(
        &mut self,
        function_id: usize,
        locals: HashMap<String, RuntimeValue>,
    ) -> Result<RuntimeValue, String> {
        let Some(function) = self.program.functions.get(function_id).cloned() else {
            return Err(format!("байт-код функции #{} не найден", function_id));
        };

        let mut stack: Vec<RuntimeValue> = Vec::new();
        let mut ip = 0usize;
        let mut steps = 0usize;

        while ip < function.code.len() {
            steps += 1;
            if steps > 200_000 {
                return Err(format!(
                    "превышен лимит инструкций при выполнении '{}'",
                    function.name
                ));
            }

            match function.code[ip].clone() {
                Instruction::PushLiteral(value) => stack.push(value),
                Instruction::LoadName(name) => {
                    let value = self.resolve_name(&name, &locals)?;
                    stack.push(value);
                }
                Instruction::BuildList(len) => {
                    if stack.len() < len {
                        return Err("недостаточно значений на стеке для сборки списка".to_string());
                    }
                    let mut items = Vec::with_capacity(len);
                    for _ in 0..len {
                        if let Some(value) = stack.pop() {
                            items.push(value);
                        }
                    }
                    items.reverse();
                    stack.push(RuntimeValue::List(items));
                }
                Instruction::Unary(op) => {
                    let Some(value) = stack.pop() else {
                        return Err(format!(
                            "стек пуст перед унарной операцией '{}' в '{}'",
                            op, function.name
                        ));
                    };
                    let result = self.eval_unary(&op, value)?;
                    stack.push(result);
                }
                Instruction::Binary(op) => {
                    let Some(right) = stack.pop() else {
                        return Err(format!(
                            "стек пуст (rhs) перед бинарной операцией '{}' в '{}'",
                            op, function.name
                        ));
                    };
                    let Some(left) = stack.pop() else {
                        return Err(format!(
                            "стек пуст (lhs) перед бинарной операцией '{}' в '{}'",
                            op, function.name
                        ));
                    };
                    let result = self.eval_binary(&op, left, right, &locals)?;
                    stack.push(result);
                }
                Instruction::MakeFunction(inner_id) => {
                    stack.push(RuntimeValue::Function(Box::new(RuntimeFunction::User {
                        function_id: inner_id,
                        captured: locals.clone(),
                        applied: Vec::new(),
                    })));
                }
                Instruction::MakeOperatorRef(op) => {
                    stack.push(RuntimeValue::Function(Box::new(RuntimeFunction::Builtin {
                        kind: BuiltinKind::OperatorRef(op),
                        applied: Vec::new(),
                    })));
                }
                Instruction::MakeSectionRight(op) => {
                    let Some(rhs) = stack.pop() else {
                        return Err(format!("стек пуст перед формированием секции '{}'", op));
                    };
                    stack.push(RuntimeValue::Function(Box::new(RuntimeFunction::Builtin {
                        kind: BuiltinKind::SectionRight {
                            op,
                            rhs: Box::new(rhs),
                        },
                        applied: Vec::new(),
                    })));
                }
                Instruction::Call => {
                    let Some(argument) = stack.pop() else {
                        return Err(format!(
                            "стек пуст (аргумент) перед вызовом функции в '{}'",
                            function.name
                        ));
                    };
                    let Some(callable) = stack.pop() else {
                        return Err(format!(
                            "стек пуст (функция) перед вызовом функции в '{}'",
                            function.name
                        ));
                    };
                    let result = self.apply_function(callable, argument)?;
                    stack.push(result);
                }
                Instruction::Return => {
                    let Some(value) = stack.pop() else {
                        return Err(format!("функция '{}' завершилась без результата", function.name));
                    };
                    if !stack.is_empty() {
                        return Err(format!(
                            "в функции '{}' стек не пуст при возврате ({} знач.)",
                            function.name,
                            stack.len()
                        ));
                    }
                    return Ok(value);
                }
            }

            ip += 1;
        }

        Err(format!(
            "функция '{}' завершилась без инструкции Return",
            function.name
        ))
    }

    fn apply_function(
        &mut self,
        callable: RuntimeValue,
        argument: RuntimeValue,
    ) -> Result<RuntimeValue, String> {
        let RuntimeValue::Function(function) = callable else {
            return Err("попытка применить как функцию нефункциональное значение".to_string());
        };

        match *function {
            RuntimeFunction::User {
                function_id,
                captured,
                mut applied,
            } => {
                applied.push(argument);
                let arity = self
                    .program
                    .functions
                    .get(function_id)
                    .map(|fn_def| fn_def.params.len())
                    .ok_or_else(|| format!("функция #{} не найдена", function_id))?;

                if applied.len() < arity {
                    return Ok(RuntimeValue::Function(Box::new(RuntimeFunction::User {
                        function_id,
                        captured,
                        applied,
                    })));
                }

                self.execute_user_function(function_id, captured, applied)
            }
            RuntimeFunction::Builtin { kind, mut applied } => {
                applied.push(argument);
                let arity = builtin_arity(&kind);

                if applied.len() < arity {
                    return Ok(RuntimeValue::Function(Box::new(RuntimeFunction::Builtin {
                        kind,
                        applied,
                    })));
                }

                self.execute_builtin(kind, applied)
            }
        }
    }

    fn execute_builtin(
        &mut self,
        kind: BuiltinKind,
        args: Vec<RuntimeValue>,
    ) -> Result<RuntimeValue, String> {
        match kind {
            BuiltinKind::OperatorRef(op) => {
                if args.len() != 2 {
                    return Err(format!(
                        "оператор '{}' ожидал 2 аргумента, получено {}",
                        op,
                        args.len()
                    ));
                }
                self.eval_binary(&op, args[0].clone(), args[1].clone(), &HashMap::new())
            }
            BuiltinKind::SectionRight { op, rhs } => {
                if args.len() != 1 {
                    return Err(format!(
                        "секция '{}' ожидала 1 аргумент, получено {}",
                        op,
                        args.len()
                    ));
                }
                self.eval_binary(&op, args[0].clone(), rhs.as_ref().clone(), &HashMap::new())
            }
            BuiltinKind::Constructor { name, .. } => Ok(RuntimeValue::Tagged { name, fields: args }),
            BuiltinKind::Show => {
                if args.len() != 1 {
                    return Err(format!("show ожидал 1 аргумент, получено {}", args.len()));
                }
                Ok(RuntimeValue::String(args[0].to_string()))
            }
            BuiltinKind::Mod => {
                if args.len() != 2 {
                    return Err(format!("mod ожидал 2 аргумента, получено {}", args.len()));
                }
                self.eval_binary("%", args[0].clone(), args[1].clone(), &HashMap::new())
            }
        }
    }

    fn eval_unary(&self, op: &str, value: RuntimeValue) -> Result<RuntimeValue, String> {
        match (op, value) {
            ("+", RuntimeValue::Int(v)) => Ok(RuntimeValue::Int(v)),
            ("+", RuntimeValue::Double(v)) => Ok(RuntimeValue::Double(v)),
            ("-", RuntimeValue::Int(v)) => Ok(RuntimeValue::Int(-v)),
            ("-", RuntimeValue::Double(v)) => Ok(RuntimeValue::Double(-v)),
            ("!", RuntimeValue::Bool(v)) => Ok(RuntimeValue::Bool(!v)),
            ("~", RuntimeValue::Int(v)) => Ok(RuntimeValue::Int(!v)),
            ("+", other) | ("-", other) => Err(format!(
                "унарный '{}' применим только к числам, получено {}",
                op, other
            )),
            ("!", other) => Err(format!("унарный '!' применим только к Bool, получено {}", other)),
            ("~", other) => Err(format!("унарный '~' применим только к Int, получено {}", other)),
            _ => Err(format!("неподдерживаемый унарный оператор '{}'", op)),
        }
    }

    fn eval_binary(
        &mut self,
        op: &str,
        left: RuntimeValue,
        right: RuntimeValue,
        locals: &HashMap<String, RuntimeValue>,
    ) -> Result<RuntimeValue, String> {
        match op {
            "+" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Int(a + b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Double((a as f64) + b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Double(a + (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Double(a + b)),
                (lhs, rhs) => Err(format!(
                    "оператор '+' применим только к числам, получены {} и {}",
                    lhs, rhs
                )),
            },
            "-" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Int(a - b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Double((a as f64) - b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Double(a - (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Double(a - b)),
                (lhs, rhs) => Err(format!(
                    "оператор '-' применим только к числам, получены {} и {}",
                    lhs, rhs
                )),
            },
            "*" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Int(a * b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Double((a as f64) * b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Double(a * (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Double(a * b)),
                (lhs, rhs) => Err(format!(
                    "оператор '*' применим только к числам, получены {} и {}",
                    lhs, rhs
                )),
            },
            "/" => match (left, right) {
                (RuntimeValue::Int(_), RuntimeValue::Int(0)) => Err("деление на ноль (Int)".to_string()),
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Int(a / b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    if b == 0.0 {
                        Err("деление на ноль (Double)".to_string())
                    } else {
                        Ok(RuntimeValue::Double((a as f64) / b))
                    }
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    if b == 0 {
                        Err("деление на ноль (Int)".to_string())
                    } else {
                        Ok(RuntimeValue::Double(a / (b as f64)))
                    }
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => {
                    if b == 0.0 {
                        Err("деление на ноль (Double)".to_string())
                    } else {
                        Ok(RuntimeValue::Double(a / b))
                    }
                }
                (lhs, rhs) => Err(format!(
                    "оператор '/' применим только к числам, получены {} и {}",
                    lhs, rhs
                )),
            },
            "%" => match (left, right) {
                (RuntimeValue::Int(_), RuntimeValue::Int(0)) => {
                    Err("остаток от деления на ноль".to_string())
                }
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Int(a % b)),
                (lhs, rhs) => Err(format!(
                    "оператор '%' поддерживает только Int, получены {} и {}",
                    lhs, rhs
                )),
            },
            "^" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) if b >= 0 => {
                    Ok(RuntimeValue::Int(a.pow(b as u32)))
                }
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Double((a as f64).powf(b as f64)))
                }
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Double((a as f64).powf(b)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Double(a.powf(b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Double(a.powf(b))),
                (lhs, rhs) => Err(format!(
                    "оператор '^' применим только к числам, получены {} и {}",
                    lhs, rhs
                )),
            },
            "==" => Ok(RuntimeValue::Bool(runtime_values_equal(&left, &right)?)),
            "/=" => Ok(RuntimeValue::Bool(!runtime_values_equal(&left, &right)?)),
            "<" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Bool(a < b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Bool((a as f64) < b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Bool(a < (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Bool(a < b)),
                (RuntimeValue::String(a), RuntimeValue::String(b)) => Ok(RuntimeValue::Bool(a < b)),
                (RuntimeValue::Char(a), RuntimeValue::Char(b)) => Ok(RuntimeValue::Bool(a < b)),
                (lhs, rhs) => Err(format!(
                    "оператор '<' не поддерживает значения {} и {}",
                    lhs, rhs
                )),
            },
            ">" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Bool(a > b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Bool((a as f64) > b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Bool(a > (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Bool(a > b)),
                (RuntimeValue::String(a), RuntimeValue::String(b)) => Ok(RuntimeValue::Bool(a > b)),
                (RuntimeValue::Char(a), RuntimeValue::Char(b)) => Ok(RuntimeValue::Bool(a > b)),
                (lhs, rhs) => Err(format!(
                    "оператор '>' не поддерживает значения {} и {}",
                    lhs, rhs
                )),
            },
            "<=" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Bool(a <= b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Bool((a as f64) <= b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Bool(a <= (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Bool(a <= b)),
                (RuntimeValue::String(a), RuntimeValue::String(b)) => Ok(RuntimeValue::Bool(a <= b)),
                (RuntimeValue::Char(a), RuntimeValue::Char(b)) => Ok(RuntimeValue::Bool(a <= b)),
                (lhs, rhs) => Err(format!(
                    "оператор '<=' не поддерживает значения {} и {}",
                    lhs, rhs
                )),
            },
            ">=" => match (left, right) {
                (RuntimeValue::Int(a), RuntimeValue::Int(b)) => Ok(RuntimeValue::Bool(a >= b)),
                (RuntimeValue::Int(a), RuntimeValue::Double(b)) => {
                    Ok(RuntimeValue::Bool((a as f64) >= b))
                }
                (RuntimeValue::Double(a), RuntimeValue::Int(b)) => {
                    Ok(RuntimeValue::Bool(a >= (b as f64)))
                }
                (RuntimeValue::Double(a), RuntimeValue::Double(b)) => Ok(RuntimeValue::Bool(a >= b)),
                (RuntimeValue::String(a), RuntimeValue::String(b)) => Ok(RuntimeValue::Bool(a >= b)),
                (RuntimeValue::Char(a), RuntimeValue::Char(b)) => Ok(RuntimeValue::Bool(a >= b)),
                (lhs, rhs) => Err(format!(
                    "оператор '>=' не поддерживает значения {} и {}",
                    lhs, rhs
                )),
            },
            "&&" => match (left, right) {
                (RuntimeValue::Bool(a), RuntimeValue::Bool(b)) => Ok(RuntimeValue::Bool(a && b)),
                (lhs, rhs) => Err(format!(
                    "оператор '&&' ожидает Bool и Bool, получены {} и {}",
                    lhs, rhs
                )),
            },
            "||" => match (left, right) {
                (RuntimeValue::Bool(a), RuntimeValue::Bool(b)) => Ok(RuntimeValue::Bool(a || b)),
                (lhs, rhs) => Err(format!(
                    "оператор '||' ожидает Bool и Bool, получены {} и {}",
                    lhs, rhs
                )),
            },
            "++" => match (left, right) {
                (RuntimeValue::String(a), RuntimeValue::String(b)) => {
                    Ok(RuntimeValue::String(format!("{}{}", a, b)))
                }
                (RuntimeValue::List(mut a), RuntimeValue::List(b)) => {
                    a.extend(b);
                    Ok(RuntimeValue::List(a))
                }
                (lhs, rhs) => Err(format!(
                    "оператор '++' поддерживает только String и List, получены {} и {}",
                    lhs, rhs
                )),
            },
            "::" => match (left, right) {
                (head, RuntimeValue::List(mut tail)) => {
                    let mut list = Vec::with_capacity(tail.len() + 1);
                    list.push(head);
                    list.append(&mut tail);
                    Ok(RuntimeValue::List(list))
                }
                (lhs, rhs) => Err(format!(
                    "оператор '::' ожидает список справа, получены {} и {}",
                    lhs, rhs
                )),
            },
            "$" => self.apply_function(left, right),
            custom_op => {
                let op_value = self.resolve_name(custom_op, locals)?;
                let first = self.apply_function(op_value, left)?;
                self.apply_function(first, right)
            }
        }
    }

    pub fn evaluate_entry(&mut self, entry: &str) -> Result<RuntimeValue, String> {
        let Some(definition) = self.program.globals.get(entry).cloned() else {
            return Err(format!("точка входа '{}' не найдена", entry));
        };

        if definition.arity != 0 {
            return Err(format!(
                "точка входа '{}' имеет арность {} (ожидается 0)",
                entry, definition.arity
            ));
        }

        self.evaluate_global_constant(entry)
    }

    pub fn evaluate_all_zero_arity(&mut self) -> Vec<(String, Result<RuntimeValue, String>)> {
        let mut ordered: Vec<(String, GlobalDefinition)> = self
            .program
            .globals
            .iter()
            .map(|(name, def)| (name.clone(), def.clone()))
            .collect();
        ordered.sort_by_key(|(_, def)| def.line);

        let mut results = Vec::new();
        for (name, def) in ordered {
            if def.arity == 0 {
                let value = self.evaluate_global_constant(&name);
                results.push((name, value));
            }
        }
        results
    }
}

pub fn print_interpreter_build_report(program: &BytecodeProgram) {
    if !program.warnings.is_empty() {
        println!();
        println!("Предупреждения сборки байт-кода:");
        for warning in &program.warnings {
            println!("- {}", warning);
        }
    }

    if !program.errors.is_empty() {
        println!();
        println!("Ошибки сборки байт-кода:");
        for error in &program.errors {
            println!("- {}", error);
        }
    }
}

pub fn print_bytecode(program: &BytecodeProgram) {
    println!();
    println!("Байт-код:");

    let mut globals: Vec<(String, GlobalDefinition)> = program
        .globals
        .iter()
        .map(|(name, def)| (name.clone(), def.clone()))
        .collect();
    globals.sort_by_key(|(_, def)| def.line);

    for (name, definition) in globals {
        let Some(function) = program.functions.get(definition.function_id) else {
            println!(
                "- {}: внутренняя ошибка (функция #{} отсутствует)",
                name, definition.function_id
            );
            continue;
        };

        let params = if function.params.is_empty() {
            "-".to_string()
        } else {
            function.params.join(", ")
        };

        println!(
            "- {} (line {}, arity {}, params: {})",
            name, definition.line, definition.arity, params
        );
        for (idx, instruction) in function.code.iter().enumerate() {
            println!("    {:>3}: {}", idx, instruction);
        }
    }
}
