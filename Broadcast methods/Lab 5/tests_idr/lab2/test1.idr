module Main

import Data.Vect

-- Single-line comment test
initialValue : Int
initialValue = 42

doubleValue : Double
doubleValue = 3.14159

hexValue : Int
hexValue = 0x1F

greeting : String
greeting = "Hello, Idris!"

myChar : Char
myChar = 'a'

main : IO ()
main = do
  putStrLn "Test program"
  let x = 10
  let y = 20
  if x < y then
    putStrLn "x is less than y"
  else
    putStrLn "x is greater or equal to y"
