module Main

import Data.Vect
import Data.List

-- 1. Переменные как неизменяемые привязки
greeting : String
greeting = "Idris"

meaningOfLife : Int
meaningOfLife = 42

-- 2. Простые типы
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

-- 3. Полиморфные типы
maybeNumber : Maybe Int
maybeNumber = Just 10

nothingValue : Maybe Int
nothingValue = Nothing

eitherValue : Either String Int
eitherValue = Right 42

-- 4. Зависимые типы
-- Вектор фиксированной длины
vectorExample : Vect 3 Int
vectorExample = [1, 2, 3]

-- Матрица как вектор векторов
matrixExample : Vect 2 (Vect 3 Int)
matrixExample = [[1, 2, 3], [4, 5, 6]]

-- 5. Алгебраические типы данных
-- Перечисление
data Color = Red | Green | Blue

implementation Show Color where
  show Red = "Красный"
  show Green = "Зеленый"
  show Blue = "Синий"

-- Произведение (запись)
record Person where
  constructor MkPerson
  name : String
  age : Int

-- Рекурсивная структура
data Tree a = Empty | Node (Tree a) a (Tree a)

-- 6. Функции
-- Простое определение
add : Int -> Int -> Int
add x y = x + y

-- Лямбда-выражение
multiplyByTwo : Int -> Int
multiplyByTwo = \x => x * 2

-- Функция высшего порядка
applyOperation : (Int -> Int) -> Int -> Int
applyOperation f x = f x

-- Зависимая функция
safeHead : Vect (S n) a -> a
safeHead (x :: xs) = x

-- 7. Рекурсия (вместо циклов)
-- Прямая рекурсия
sumList : List Int -> Int
sumList [] = 0
sumList (x :: xs) = x + sumList xs

-- Хвостовая рекурсия
productList : List Int -> Int
productList xs = go xs 1
  where
    go : List Int -> Int -> Int
    go [] acc = acc
    go (y :: ys) acc = go ys (y * acc)

-- 8. Функции высшего порядка
doubleList : List Int -> List Int
doubleList = map (*2)

filterEven : List Int -> List Int
filterEven = filter (\x => x `mod` 2 == 0)

-- 9. Условные операторы
-- if...then...else
checkSign : Int -> String
checkSign x = if x > 0 
              then "Положительное" 
              else if x < 0 
                   then "Отрицательное" 
                   else "Ноль"

-- case...of
describeList : List a -> String
describeList xs = case xs of
  [] => "Пустой список"
  [x] => "Список с одним элементом"
  (x :: y :: []) => "Список с двумя элементами"
  _ => "Список с тремя или более элементами"

-- 10. Натуральные числа
natZero : Nat
natZero = Z

natThree : Nat
natThree = S (S (S Z))

-- 11. Типы функций
intToString : Int -> String
intToString = show

composeFunctions : (b -> c) -> (a -> b) -> (a -> c)
composeFunctions f g = \x => f (g x)

main : IO ()
main = do
  putStrLn "=== Программа 2: Основы Idris ==="
  
  -- Демонстрация переменных
  putStrLn $ "Приветствие: " ++ greeting
  putStrLn $ "Смысл жизни: " ++ show meaningOfLife
  
  -- Демонстрация типов
  putStrLn $ "Int: " ++ show intValue
  putStrLn $ "Double: " ++ show doubleValue
  putStrLn $ "Char: " ++ show charValue
  putStrLn $ "String: " ++ stringValue
  putStrLn $ "Bool: " ++ show boolValue
  
  -- Демонстрация полиморфных типов
  putStrLn $ "Maybe Int: " ++ show maybeNumber
  putStrLn $ "Either String Int: " ++ show eitherValue
  
  -- Демонстрация зависимых типов
  putStrLn $ "Вектор: " ++ show vectorExample
  putStrLn $ "Первый элемент вектора: " ++ show (safeHead vectorExample)
  
  -- Демонстрация алгебраических типов
  putStrLn $ "Цвет: " ++ show Red
  let person = MkPerson "Иван" 30
  putStrLn $ "Человек: " ++ person.name ++ ", возраст: " ++ show person.age
  
  -- Демонстрация функций
  putStrLn $ "Сложение: 5 + 3 = " ++ show (add 5 3)
  putStrLn $ "Лямбда: 10 * 2 = " ++ show (multiplyByTwo 10)
  putStrLn $ "ФВП: (*2)(21) = " ++ show (applyOperation (*2) 21)
  
  -- Демонстрация рекурсии
  let numbers = [1, 2, 3, 4, 5]
  putStrLn $ "Сумма списка " ++ show numbers ++ " = " ++ show (sumList numbers)
  putStrLn $ "Произведение списка = " ++ show (productList numbers)
  
  -- Демонстрация функций высшего порядка
  putStrLn $ "Удвоенный список: " ++ show (doubleList numbers)
  putStrLn $ "Четные числа: " ++ show (filterEven numbers)
  
  -- Демонстрация условных операторов
  putStrLn $ "Знак числа 42: " ++ checkSign 42
  putStrLn $ "Описание списка: " ++ describeList [1, 2, 3]
  
  -- Демонстрация натуральных чисел
  putStrLn $ "Натуральное число 3: " ++ show natThree
  
  -- Демонстрация типов функций
  putStrLn $ "Int в строку: " ++ intToString 123
  putStrLn $ "Композиция функций: " ++ show (composeFunctions (+1) (*2) 5)
  
  putStrLn "=== Программа 2 завершена ==="
