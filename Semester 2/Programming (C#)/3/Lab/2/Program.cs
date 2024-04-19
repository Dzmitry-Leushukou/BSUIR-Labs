using _2;
using _2.Services;
using System.ComponentModel.Design;

internal class Program
{
    private static void Main(string[] args)
    {
        int z,k = 0;
        while (true)
        {
            Console.WriteLine("Write z and k");
            if (int.TryParse(Console.ReadLine(), out z)&&int.TryParse(Console.ReadLine(), out k))
           {
                MathFunc f = new MathFunc(z, k);
                f.get();
            }
            
        }
    }
}