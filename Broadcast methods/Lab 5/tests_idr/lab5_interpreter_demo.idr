module Lab5InterpreterDemo

x : Int
x = 10

y : Int
y = 5

add : Int -> Int -> Int
add a b = a + b

twice : Int -> Int
twice = \n => n * 2

numbers : List Int
numbers = [1, 2, 3]

prepended : List Int
prepended = 0 :: numbers

joined : List Int
joined = prepended ++ [4, 5]

check : Bool
check = (add x y) == 15 && True

main : String
main = "result=" ++ show (twice (add x y))
