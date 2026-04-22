-- Comprehensive test demonstrating all lexer features

module ComprehensiveTest

import Data.List
import Data.Nat

{- Multi-line comment demonstrating nesting
   {- Level 1 nesting -}
   {- Another level 1 -}
   Still in outer comment
-}

-- Keywords demonstration
data Color = Red | Green | Blue

interface Printable a where
  print : a -> String

implementation Printable Int where
  print n = show n

-- Type annotations with various operators
myFunc : Nat -> Nat -> Nat
myFunc x y = x + y

-- Identifiers with underscores and digits
my_var1 : Int
my_var1 = 42

anotherVar2 : Int
anotherVar2 = 100

-- Numeric constants
intTest : Int
intTest = 123

negInt : Int
negInt = -456

decimal : Double
decimal = 3.14159265

scientific1 : Double
scientific1 = 1e-10

scientific2 : Double
scientific2 = 2.5E+3

scientific3 : Double
scientific3 = 3.14e5

hexNumber : Int
hexNumber = 0x1F

hexUpper : Int
hexUpper = 0XAB

hexLarge : Int
hexLarge = 0xDEADBEEF

-- String constants with escapes
normalString : String
normalString = "Hello World"

stringWithNewline : String
stringWithNewline = "Line1\nLine2"

stringWithTab : String
stringWithTab = "Column1\tColumn2"

stringWithBackslash : String
stringWithBackslash = "Path\\to\\file"

stringWithQuotes : String
stringWithQuotes = "Say \"Hello\""

emptyString : String
emptyString = ""

-- Character constants with escapes
simpleChar : Char
simpleChar = 'a'

charNewline : Char
charNewline = '\n'

charTab : Char
charTab = '\t'

charBackslash : Char
charBackslash = '\\'

charQuote : Char
charQuote = '\''

-- Operators demonstration
addition : Int
addition = 1 + 2

subtraction : Int
subtraction = 5 - 3

multiplication : Int
multiplication = 4 * 3

division : Int
division = 10 / 2

modulo : Int
modulo = 7 % 3

power : Int
power = 2 ^ 8

-- Comparison operators
eqTest : Bool
eqTest = 1 == 1

neTest : Bool
neTest = 1 /= 2

ltTest : Bool
ltTest = 1 < 2

gtTest : Bool
gtTest = 2 > 1

leTest : Bool
leTest = 2 <= 2

geTest : Bool
geTest = 2 >= 1

-- Logical operators
andTest : Bool
andTest = True && False

orTest : Bool
orTest = True || False

bangTest : Bool
bangTest = !True

-- Special Idris operators
colonOp : a -> b
colonOp : a -> b

doubleColon : ()
doubleColon = ()

arrowOp : a -> b
arrowOp = \x => x

fatArrowOp : a => b
fatArrowOp = True

backslashLam : Int -> Int
backslashLam = \x => x + 1

pipeOp : Int
pipeOp = 1 | 2

dollarFunc : Int
dollarFunc = id $ 42

tildeOp : ()
tildeOp = ()

questionMark : ()
questionMark = ()

dotDotOp : ()
dotDotOp = 1 .. 10

-- Delimiters and punctuation
parens : (Int)
parens = (42)

brackets : [Int]
brackets = [1, 2, 3]

braces : {}
braces = {}

comma_test : (Int, String)
comma_test = (42, "test")

semicolon_test : Int
semicolon_test = 42

backtick_test : Int
backtick_test = `add` 1 2

-- Case expression
caseExpr : Int -> String
caseExpr n = case n of
  0 => "Zero"
  1 => "One"
  _ => "Other"

-- If-then-else
ifExpr : String
ifExpr = if True then "yes" else "no"

-- Let expression
letExpr : Int
letExpr = let x = 5 in x + x

-- Lambda in do notation
doExpr : IO ()
doExpr = do
  putStrLn "Testing"
  let f = \x => x * 2
  pure ()

-- Record
record Point where
  x : Int
  y : Int

-- Forall
forallTest : forall a. a -> a
forallTest x = x

-- Export/private/public
export
publicFunc : Int
publicFunc = 42

private
privateFunc : String
privateFunc = "hidden"

-- Using and parameters
using (n : Nat)
  foo : Vec n Int
  foo = replicate 0

-- Namespace
namespace MyNamespace
  myFunc2 : Int
  myFunc2 = 100
