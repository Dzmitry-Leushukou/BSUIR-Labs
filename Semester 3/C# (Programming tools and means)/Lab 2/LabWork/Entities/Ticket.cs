using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _353504_Levshukov_Lab5.Entities
{
    internal class Ticket
    {
        public string Route { get; private set; }
        public string? Date{ get; set; }
        public double Price { get; private set; }

        public Ticket(string route,double price, string? date = null)
        { 
           Route = route;
           Date = date;
           Price = price;
        }

        public override string ToString()
        {
            return $"[{Date}] {Date} {Price}$";
        }
    }
}
