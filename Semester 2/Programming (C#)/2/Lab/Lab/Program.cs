using System.ComponentModel.Design;

internal class Program
{
    public static int Menu()
    {
        Console.WriteLine("1. Продолжить\n2. Завершить");
        Console.Write("Выберите пункт меню введя его номер: ");

        int number = -1;
        int.TryParse(Console.ReadLine(), out number);

        switch(number)
        {
            case 1: 
                return number;
            case 2: 
                return number;
            default:
                Console.WriteLine("Некорректный ввод");
                return Menu();
        }
        
    }
    private static void Main(string[] args)
    {
        int a, b, c;
        while (true)
        {
            Console.WriteLine("Введите возможные стороны (натуральные числа) треугольника (a,b,c) через Enter");

            Console.Write("a = ");
            int.TryParse(Console.ReadLine(), out a);

            Console.Write("b = ");
            int.TryParse(Console.ReadLine(), out b);

            Console.Write("с = ");
            int.TryParse(Console.ReadLine(), out c);

            if (a <= 0 || b <= 0 || c <= 0)
            {
                Console.WriteLine("Некорректные данные");
            
                if (Menu() == 2)
                    return;

                continue;
            }

            if (a + b < c || b + c < a || c + a < b)
            {
                Console.WriteLine("Не является сторонами треугольника");
                if (Menu() == 2)
                    return;
                continue;
            }
            a *= a;
            b *= b;
            c *= c;

            if (a + b == c)
                Console.WriteLine("Являются сторонами прямоугольного треугольника a,b - катеты, с - гипотенуза");
            else
            if (a + c == b)
                Console.WriteLine("Являются сторонами прямоугольного треугольника a,c - катеты, b - гипотенуза");
            else
            if (b + c == a)
                Console.WriteLine("Являются сторонами прямоугольного треугольника b,c - катеты, a - гипотенуза");
            else
                Console.WriteLine("Не являются сторонами прямоугольного треугольника");               

            if (Menu() == 2)
                return;
        }
    }


}