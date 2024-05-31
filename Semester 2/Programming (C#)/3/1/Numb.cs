using System.Security.Cryptography.X509Certificates;

namespace Lab
{
    internal class Numb
    {
        private int n;
        public Numb(int n1)
        {
            n = n1;
        }
        public void change()
        {
            if (n % 2 == 0)
                n /= 2;
            else
            n= 0;
        }

        public int get()
        {
            return n;
        }
    }
}
