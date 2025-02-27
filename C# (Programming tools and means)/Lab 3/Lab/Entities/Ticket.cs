using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class Ticket
    {
        public Tariff Tariff { get; private set; }
        public Passenger owner;
        public string Date { get; private set; }
        public Ticket(Tariff tariff,string date, Passenger owner)
        {

            Tariff = tariff;
            this.owner = owner;
            Date = date;
        }
        public override string ToString() 
        {
            return $"{Tariff.Route}  Owner: {owner.ToString()}";
        }
    }
}
