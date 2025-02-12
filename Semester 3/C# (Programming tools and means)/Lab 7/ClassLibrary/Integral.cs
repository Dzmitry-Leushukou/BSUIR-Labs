using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices.Marshalling;
using System.Security.Principal;
using System.Threading;

namespace ClassLibrary
{
    public class Integral
    {
        public delegate void ShowTicks(Int64 ticks, int thread, double res);
        public event ShowTicks? EndIntegral;

        public delegate void ShowProc(int p,int numb);
        public event ShowProc? ProcIntegral;
        public void GetIntegral()
        {
            Stopwatch sw = Stopwatch.StartNew();
            double step = 0.00000001, ans = 0;
            int last=-1;
            for(double i=0;i<=1;i+=step)
            {
                ans += Math.Sin(i)*step;
               
                if(last!=(int)(i*100))
                {
                    last = (int)(i*100);
                    ProcIntegral?.Invoke((int)(i * 100), Thread.CurrentThread.ManagedThreadId);
                }
                
                //wait(10);
            }
            sw.Stop();
            EndIntegral?.Invoke(sw.ElapsedTicks, Thread.CurrentThread.ManagedThreadId, ans);
        }

        void wait(int x)
        {
            int a;
            for (int i = 0; i < x; i++)
            a = 2*2;
        }
    }
}
