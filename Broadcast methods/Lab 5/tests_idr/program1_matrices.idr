module Main

import Data.Vect
import Data.String

initialValue : Int
initialValue = 42

doubleValue : Double
doubleValue = 3.14159

hexValue : Int
hexValue = 0x1F

greeting : String
greeting = "Hello, Idris!"

data Matrix : (rows : Nat) -> (cols : Nat) -> Type -> Type where
    MkMatrix : Vect rows (Vect cols a) -> Matrix rows cols a

data Operation = Add | Subtract | Multiply | Transpose

exampleMatrix : Matrix 2 2 Int
exampleMatrix = MkMatrix [[1, 2], [3, 4]]

zeros : (rows : Nat) -> (cols : Nat) -> Matrix rows cols Int
zeros rows cols = MkMatrix $ replicate rows (replicate cols 0)

matrixMap : (a -> b) -> Matrix rows cols a -> Matrix rows cols b
matrixMap f (MkMatrix xs) = MkMatrix $ map (map f) xs

sumMatrix : Num a => Matrix rows cols a -> a
sumMatrix (MkMatrix []) = 0
sumMatrix (MkMatrix (row :: rows)) = sum row + sumMatrix (MkMatrix rows)

describeOperation : Operation -> String
describeOperation op = case op of
    Add => "Сложение матриц"
    Subtract => "Вычитание матриц"
    Multiply => "Умножение матриц"
    Transpose => "Транспонирование матрицы"

isSquareMatrix : Matrix rows cols a -> Bool
isSquareMatrix _ = True  

scaleMatrix : Num a => a -> Matrix rows cols a -> Matrix rows cols a
scaleMatrix factor = matrixMap (\x => x * factor)

main : IO ()
main = do
  putStrLn "=== Программа 1: Работа с матрицами ==="
  putStrLn $ "Числовые константы: Int=" ++ show initialValue ++
             ", Double=" ++ show doubleValue ++
             ", Hex=" ++ show hexValue
  putStrLn $ "Текстовая константа: " ++ greeting
  putStrLn $ "Сумма матрицы: " ++ show (sumMatrix exampleMatrix)
  putStrLn $ "Операция: " ++ describeOperation Add
  putStrLn $ "Квадратная ли матрица: " ++ show (isSquareMatrix exampleMatrix)
  let scaled = scaleMatrix 2 exampleMatrix
  putStrLn $ "Удвоенная матрица (сумма): " ++ show (sumMatrix scaled)
