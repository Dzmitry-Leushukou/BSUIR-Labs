{- Nested comment test
   {- Inner comment -}
   Still in outer comment
-}

module Advanced

data Tree a = Leaf a | Node (Tree a) (Tree a)

-- Test escape sequences in strings
testStr : String
testStr = "Line1\nLine2\tTabbed\"Quoted\""

-- Test escape sequences in chars
testChar : Char
testChar = '\n'

-- Test scientific notation
scientific1 : Double
scientific1 = 1.5e-10

scientific2 : Double  
scientific2 = 2.5E+3

-- Lambda expression with operators
lambda : Int -> Int -> Int
lambda = \x => \y => x + y * 2 - y / 2

-- Various operators
ops : Int
ops = 1 + 2 * 3 ^ 4 - 5 % 3

-- Comparison operators
cmp : Bool
cmp = (1 < 2) && (3 >= 2) || (5 /= 4)

-- More operators
dotOp : ()
dotOp = 1 .. 10

-- Fat arrow and arrow
fatArrow => val
arrow : a -> b

record Point where
  x : Int
  y : Int
