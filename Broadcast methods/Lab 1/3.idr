module Main

import Data.Vect
import Data.List
import Data.String
import Data.Stream

-- 1. Символы и строки (Char, String)
textDemo : IO ()
textDemo = do
  putStrLn $ "Символ 'a': " ++ show 'a'
  putStrLn $ "Символ новой строки: \\n (ASCII " ++ show (ord '\n') ++ ")"
  putStrLn $ "Строковая константа: \"Hello, Idris!\""

-- 2. Типы функций и лямбда-выражения
applyFunction : (Int -> String) -> Int -> String
applyFunction f x = f x

lambdaExample : Int -> String
lambdaExample = \x => "Число: " ++ show x

-- 3. Простые типы
simpleTypes : IO ()
simpleTypes = do
  let intVal : Int = 42
  let doubleVal : Double = 2.0e-5  -- Научная нотация
  let boolVal : Bool = True
  putStrLn $ "Int: " ++ show intVal
  putStrLn $ "Double (2.0e-5): " ++ show doubleVal
  putStrLn $ "Bool: " ++ show boolVal

-- 4. Функции высшего порядка (foldr, map, traverse)
sumList : List Int -> Int
sumList = foldr (+) 0

-- traverse пример
traverseExample : IO ()
traverseExample = do
  let list = the (List Int) [1, 2, 3]
  let maybeList = traverse (\x => if x > 0 then Just x else Nothing) list
  putStrLn $ "traverse проверки положительных: " ++ show maybeList

-- Функции высшего порядка из текста
demoHigherOrder : IO ()
demoHigherOrder = do
  let numbers = [1, 2, 3, 4, 5]
  let doubled = map (*2) numbers
  let filtered = Prelude.List.filter (\x => x > 3) numbers
  
  putStrLn $ "map (*2) [1,2,3,4,5] = " ++ show doubled
  putStrLn $ "filter (>3) [1,2,3,4,5] = " ++ show filtered
  
  -- foldl vs foldr
  let leftSum = foldl (+) 0 numbers
  let rightSum = foldr (+) 0 numbers
  putStrLn $ "foldl (+) 0 [1,2,3,4,5] = " ++ show leftSum
  putStrLn $ "foldr (+) 0 [1,2,3,4,5] = " ++ show rightSum

-- 5. Условные операторы: if...then...else и case...of
demoConditionals : IO ()
demoConditionals = do
  -- if...then...else
  let x = 10
  let result = if x > 5 then "Больше 5" else "Меньше или равно 5"
  putStrLn $ "if 10 > 5 then...: " ++ result
  
  -- case...of с деструктуризацией сложных типов
  let list = the (List Int) [1, 2, 3]
  let description = case list of
                     [] => "Пустой список"
                     [x] => "Список с одним элементом: " ++ show x
                     (x :: y :: xs) => "Список минимум из двух элементов, первый: " ++ show x
  
  putStrLn $ "case для списка [1,2,3]: " ++ description

-- 6. Натуральные числа (Z, S) и зависимые типы
natExample : Nat
natExample = S (S (S Z))  -- 3

-- Вспомогательная функция
natToInt : Nat -> Int
natToInt Z = 0
natToInt (S k) = 1 + natToInt k

-- Зависимый тип: вектор фиксированной длины
safeHead : Vect (S n) a -> a
safeHead (x :: xs) = x

-- 7. Императивный цикл через рекурсию
simulateWhile : Nat -> IO ()
simulateWhile n = go 1
  where
    go : Nat -> IO ()
    go i = if i <= n
           then do putStrLn $ "Итерация " ++ show i
                   go (i + 1)
           else pure ()

-- 8. Определение функций разных видов
-- Явное определение
add : Int -> Int -> Int
add x y = x + y

-- Функция с зависимыми типами
replicateVec : (n : Nat) -> a -> Vect n a
replicateVec Z _ = []
replicateVec (S k) x = x :: replicateVec k x

-- 9. do-нотация для последовательных вычислений
sequenceDemo : IO ()
sequenceDemo = do
  putStrLn "Введите число:"
  input <- getLine
  case parseInteger input of
       Just n => putStrLn $ "Вы ввели: " ++ show n
       Nothing => putStrLn "Это не число!"

-- 10. Полиморфные типы в действии
polymorphicDemo : IO ()
polymorphicDemo = do
  -- Maybe тип
  let maybeValue : Maybe Int = Just 42
  let nothingValue : Maybe Int = Nothing
  
  putStrLn $ "Maybe Int: Just 42 = " ++ show maybeValue
  putStrLn $ "Maybe Int: Nothing = " ++ show nothingValue
  
  -- Either тип
  let eitherValue : Either String Int = Right 100
  let errorValue : Either String Int = Left "Ошибка"
  
  putStrLn $ "Either String Int: Right 100 = " ++ show eitherValue
  putStrLn $ "Either String Int: Left 'Ошибка' = " ++ show errorValue

-- 11. Пример рекурсии (замена цикла)
factorial : Nat -> Nat
factorial Z = 1
factorial (S n) = (S n) * factorial n

-- 12. iterate - генерация бесконечных списков
generateInfinite : Nat -> Stream Nat
generateInfinite start = iterate (+1) start

-- 13. Хвостовая рекурсия
factorialTail : Nat -> Nat
factorialTail n = go n 1
  where
    go : Nat -> Nat -> Nat
    go Z acc = acc
    go (S k) acc = go k (S k * acc)

-- 14. Определение алгебраических типов данных (дополнение к программам 1 и 2)
-- Перечисление
data Day = Monday | Tuesday | Wednesday | Thursday | Friday | Saturday | Sunday

implementation Show Day where
  show Monday = "Понедельник"
  show Tuesday = "Вторник"
  show Wednesday = "Среда"
  show Thursday = "Четверг"
  show Friday = "Пятница"
  show Saturday = "Суббота"
  show Sunday = "Воскресенье"

-- Произведение типов
data MyPair : Type -> Type -> Type where
  MkMyPair : a -> b -> MyPair a b

implementation (Show a, Show b) => Show (MyPair a b) where
  show (MkMyPair x y) = "(" ++ show x ++ ", " ++ show y ++ ")"

-- Рекурсивная структура данных (дополнение)
data BinaryTree a = Leaf | Node (BinaryTree a) a (BinaryTree a)

-- 15. Зависимые типы функций
-- Функция, возвращающая тип, зависящий от значения
intOrString : Bool -> Type
intOrString True = Int
intOrString False = String

getValue : (b : Bool) -> intOrString b
getValue True = 42
getValue False = "Сорок два"

-- 16. Пример доказательства через case
isNonEmpty : Vect n a -> Bool
isNonEmpty [] = False
isNonEmpty (x :: xs) = True

main : IO ()
main = do
  putStrLn "=== Программа 3: Базовые конструкции ==="
  
  -- 1. Простые типы
  simpleTypes
  
  -- 2. Символы и строки
  textDemo
  
  -- 3. Натуральные числа
  putStrLn $ "Натуральное число: S (S (S Z)) = " ++ show (natToInt natExample)
  
  -- 4. Типы функций
  putStrLn $ "Тип функции: Int -> String"
  putStrLn $ "Результат: " ++ applyFunction lambdaExample 42
  
  -- 5. Функции высшего порядка
  putStrLn $ "foldr (+) 0 [1,2,3] = " ++ show (sumList [1,2,3])
  demoHigherOrder
  traverseExample
  
  -- 6. Условные операторы
  demoConditionals
  
  -- 7. Зависимые типы
  let vec = the (Vect 3 Int) [1, 2, 3]
  putStrLn $ "safeHead [1,2,3] = " ++ show (safeHead vec)
  
  -- 8. Определение функций
  putStrLn $ "add 5 3 = " ++ show (add 5 3)
  putStrLn $ "replicateVec 3 'a' = " ++ show (replicateVec 3 'a')
  
  -- 9. Полиморфные типы
  polymorphicDemo
  
  -- 10. Рекурсия
  putStrLn $ "Факториал 5 (обычная рекурсия) = " ++ show (factorial 5)
  putStrLn $ "Факториал 5 (хвостовая рекурсия) = " ++ show (factorialTail 5)
  
  -- 11. Цикл
  putStrLn "Эмуляция цикла while:"
  simulateWhile 3
  
  -- 12. iterate
  putStrLn $ "Первые 5 чисел начиная с 1: " ++ show (take 5 (Stream.take 100 (generateInfinite 1)))
  
  -- 13. Алгебраические типы данных
  putStrLn $ "День недели: " ++ show Tuesday
  putStrLn $ "Пара: " ++ show (the (MyPair Int String) (MkMyPair 1 "один"))
  
  -- 14. Зависимые типы в функциях
  putStrLn $ "Зависимый тип функции при True: " ++ show (getValue True)
  putStrLn $ "Зависимый тип функции при False: " ++ show (getValue False)
  
  -- 15. do-нотация
  sequenceDemo
  
  putStrLn "=== Программа 3 завершена ==="