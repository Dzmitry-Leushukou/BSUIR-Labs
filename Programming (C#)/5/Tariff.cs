using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    internal class Tariff
    {
        private double cost;
        public double Cost
        {
            get { return cost; } set { cost = value; }
        }
        public Tariff(double t)
        {
            cost = t;
        }
    }
}
