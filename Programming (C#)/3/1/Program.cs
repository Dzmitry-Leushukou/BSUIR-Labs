using Microsoft.VisualBasic.FileIO;

namespace Lab
{
    internal class Program
    {
        private static int Main(string[] args)
        {
            int number = 0;
            while (true)
            {
                Console.WriteLine("Write a number(>0)");
                if (!int.TryParse(Console.ReadLine(), out number) || number <= 0)
                {
                    Console.WriteLine("Wrong input");
                    continue;
                }
                Numb n = new Numb(number);

                while (n.get() > 0)
                {
                    n.change();
                    Console.WriteLine(n.get());
                }
            }
            return 0;
        }
    }
}