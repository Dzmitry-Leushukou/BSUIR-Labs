using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    public class Listener
    {
        public int Age { get; private set; }
        public bool Online { get; private set; }
        public string Name { get; private set; }

        public Listener(string name, int age, bool online)
        {
            Name = name;
            Age = age;
            Online = online;
        }
    }
}
