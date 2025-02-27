using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class Tariff
    {
        public string Route { get; private set; }
        public double Price { get; private set; }

        public Tariff(string route, double price)
        {
            Route = route;
            Price = price;
        }


    }
}
