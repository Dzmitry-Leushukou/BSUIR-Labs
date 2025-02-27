using ClassLibrary;
using System.Collections.Specialized;
internal class Program
{
    private static void Main(string[] args)
    {
        Integral integral1 = new();
        IntegralMutex integral2 = new();
        IntegralSema integral3 = new();

        integral1.EndIntegral += UI.Found;
        integral1.ProcIntegral += UI.Process;
        integral2.EndIntegral += UI.Found;
        integral2.ProcIntegral += UI.Process;
        integral3.EndIntegral += UI.Found;
        integral3.ProcIntegral += UI.Process;

        //Just
        Thread thread1 = new Thread(new ThreadStart(integral1.GetIntegral));
        Thread thread2 = new Thread(new ThreadStart(integral1.GetIntegral));

        thread1.Priority = ThreadPriority.Highest;
        thread2.Priority = ThreadPriority.Lowest;

        thread1.Start();
        thread2.Start();

        thread1.Join();
        thread2.Join();
        Console.WriteLine("\n\n\nMutex\n\n\n");
        //Mutex
        Thread[] threads = new Thread[5]; 
        for(int  i = 0; i < 5;i++)
        {
            threads[i] = new Thread(new ThreadStart(integral2.GetIntegral));
            threads[i].Start();
        }

        for (int i = 0; i < 5; i++)
        {
            threads[i].Join();
        }
        Console.WriteLine("\n\n\nSeman\n\n\n");

        //Sema
        for (int i = 0; i < 5; i++)
        {
            Thread thread = new Thread(new ThreadStart(integral3.GetIntegral));
            thread.Start();
        }
    }
}