using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ClassLibrary
{
    internal class UI
    {
        public static void Found(Int64 ticks, int thread, double res)
        {
            Console.WriteLine($"Thread: {thread} Integral was find: {res} by {ticks} ticks");
        }
        public static void Process(int per, int thread)
        {
            lock (ConsoleLock)
            {
                Console.Write($"Thread: {thread} [");
                for (int i = 0; i <= per / 10 - 1; i++)
                {
                    Console.Write("=");
                }
                Console.Write(">");
                for (int i = 0; i < 10 - per / 10; i++)
                {
                    Console.Write(" ");
                }
                Console.Write($"] {per}%\n");
            }
        }
        private static readonly object ConsoleLock = new object();
    }
}
