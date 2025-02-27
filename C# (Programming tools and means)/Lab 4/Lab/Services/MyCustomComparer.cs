using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    internal class MyCustomComparer : IComparer<Listener>
    {
        public int Compare(Listener x, Listener y) => string.Compare(x.Name, y.Name, StringComparison.Ordinal);
    }


}
