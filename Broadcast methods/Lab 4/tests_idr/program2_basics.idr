module Main

import Data.Vect
import Data.List

greeting : String
greeting = "Idris"

meaningOfLife : Int
meaningOfLife = 42

intValue : Int
intValue = -5

doubleValue : Double
doubleValue = 3.14

charValue : Char
charValue = 'a'

stringValue : String
stringValue = "Hello, Idris!"

boolValue : Bool
boolValue = True

maybeNumber : Maybe Int
maybeNumber = Just 10

nothingValue : Maybe Int
nothingValue = Nothing

eitherValue : Either String Int
eitherValue = Right 42

vectorExample : Vect 3 Int
vectorExample = [1, 2, 3]

matrixExample : Vect 2 (Vect 3 Int)
matrixExample = [[1, 2, 3], [4, 5, 6]]

data Color = Red | Green | Blue

implementation Show Color where
  show Red = "Красный"
  show Green = "Зеленый"
  show Blue = "Синий"

record Person where
  constructor MkPerson
  name : String
  age : Int

data Tree a = Empty | Node (Tree a) a (Tree a)

add : Int -> Int -> Int
add x y = x + y

multiplyByTwo : Int -> Int
multiplyByTwo = \x => x * 2

applyOperation : (Int -> Int) -> Int -> Int
applyOperation f x = f x

safeHead : Vect (S n) a -> a
safeHead (x :: xs) = x

sumList : List Int -> Int
sumList [] = 0
sumList (x :: xs) = x + sumList xs

productList : List Int -> Int
productList xs = go xs 1
  where
    go : List Int -> Int -> Int
    go [] acc = acc
    go (y :: ys) acc = go ys (y * acc)

doubleList : List Int -> List Int
doubleList = map (*2)

filterEven : List Int -> List Int
filterEven = filter (\x => x `mod` 2 == 0)

checkSign : Int -> String
checkSign x = if x > 0 
              then "Положительное" 
              else if x < 0 
                   then "Отрицательное" 
                   else "Ноль"

describeList : List a -> String
describeList xs = case xs of
  [] => "Пустой список"
  [x] => "Список с одним элементом"
  (x :: y :: []) => "Список с двумя элементами"
  _ => "Список с тремя или более элементами"

natZero : Nat
natZero = Z

natThree : Nat
natThree = S (S (S Z))

intToString : Int -> String
intToString = show

composeFunctions : (b -> c) -> (a -> b) -> (a -> c)
composeFunctions f g = \x => f (g x)

main : IO ()
main = do
  putStrLn "=== Программа 2: Основы Idris ==="
  
  putStrLn $ "Приветствие: " ++ greeting
  putStrLn $ "Смысл жизни: " ++ show meaningOfLife
  
  putStrLn $ "Int: " ++ show intValue
  putStrLn $ "Double: " ++ show doubleValue
  putStrLn $ "Char: " ++ show charValue
  putStrLn $ "String: " ++ stringValue
  putStrLn $ "Bool: " ++ show boolValue
  
  putStrLn $ "Maybe Int: " ++ show maybeNumber
  putStrLn $ "Either String Int: " ++ show eitherValue
  
  putStrLn $ "Вектор: " ++ show vectorExample
  putStrLn $ "Первый элемент вектора: " ++ show (safeHead vectorExample)
  
  putStrLn $ "Цвет: " ++ show Red
  let person = MkPerson "Иван" 30
  putStrLn $ "Человек: " ++ person.name ++ ", возраст: " ++ show person.age
  
  putStrLn $ "Сложение: 5 + 3 = " ++ show (add 5 3)
  putStrLn $ "Лямбда: 10 * 2 = " ++ show (multiplyByTwo 10)
  putStrLn $ "ФВП: (*2)(21) = " ++ show (applyOperation (*2) 21)
  
  let numbers = [1, 2, 3, 4, 5]
  putStrLn $ "Сумма списка " ++ show numbers ++ " = " ++ show (sumList numbers)
  putStrLn $ "Произведение списка = " ++ show (productList numbers)
  
  putStrLn $ "Удвоенный список: " ++ show (doubleList numbers)
  putStrLn $ "Четные числа: " ++ show (filterEven numbers)
  
  putStrLn $ "Знак числа 42: " ++ checkSign 42
  putStrLn $ "Описание списка: " ++ describeList [1, 2, 3]
  
  putStrLn $ "Натуральное число 3: " ++ show natThree
  
  putStrLn $ "Int в строку: " ++ intToString 123
  putStrLn $ "Композиция функций: " ++ show (composeFunctions (+1) (*2) 5)
  
  putStrLn "=== Программа 2 завершена ==="
