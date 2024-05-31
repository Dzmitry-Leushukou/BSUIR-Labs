internal class Program
{
    public readonly int Age;
    public static int Menu()
    {
        Console.WriteLine("1. Продолжить\n2. Завершить");
        Console.Write("Выберите пункт меню введя его номер: ");

        int number = -1;
        int.TryParse(Console.ReadLine(), out number);
        
        switch (number)
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
        int x, y;
        while (true)
        {
            Console.WriteLine("Введите координаты точки (x,y) через Enter");

            Console.Write("x = ");
            if (int.TryParse(Console.ReadLine(), out x) == false)
            {
                Console.WriteLine("Некорректные данные");
                if (Menu() == 2)
                    return;
                continue;
            }

            Console.Write("y = ");
            if (int.TryParse(Console.ReadLine(), out y) == false)
            {
                Console.WriteLine("Некорректные данные");
                if (Menu() == 2)
                    return;
                continue;
            }

            if (y < 0)
            {
                Console.WriteLine("Нет");
                if (Menu() == 2)
                    return;
                continue;
            }

            if (y == 0 && x >= -3 && x <= 3)
            {
                Console.WriteLine("На границе");
                if (Menu() == 2)
                    return;
                continue;
            }

            x *= x;
            y *= y;

            if(x+y==9)
            {
                Console.WriteLine("На границе");
                if (Menu() == 2)
                    return;
                continue;
            }

            if(x+y<9)
            {
                Console.WriteLine("Да");
                if (Menu() == 2)
                    return;
                continue;
            }

            Console.WriteLine("Нет");
            if (Menu() == 2)
                return;
            continue;

        }
    }
}