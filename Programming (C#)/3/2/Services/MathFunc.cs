using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _2.Services
{
    internal class MathFunc
    {
        private int k,z,x,branch=0;
        
        public MathFunc(int z1,int k1)
        {
            z = z1;
            k= k1;
            setx();
        }

        private void setx()
        {
            if(k<1)
            {
                x = k * z * z * z; 
                branch = 1;
            }
            else
            {
                x = z*(z + 1);
                branch = 2;
            }
        }

        public void get()
        {
            Console.WriteLine( Math.Pow(Math.Log(1 + x * x)+Math.Cos(x+1),Math.Exp(k*x)));
            //Console.Write("dsadsadsd");
        }
        static void adsd() {
            k = 1;
        }

    }
}
