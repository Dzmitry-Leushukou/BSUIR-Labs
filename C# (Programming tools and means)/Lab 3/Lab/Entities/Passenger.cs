using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class Passenger
    {
        public string PassportId { get; private set; }
        public string Name { get; private set; }
        public string Country { get; private set; }
        public List<Ticket> Tickets=new();
        public Passenger(string Name,string PassportId, string Country) 
        {
            this.PassportId=PassportId;
            this.Name=Name;
            this.Country=Country;
        }
        public double SumOfTickets()
        {
            double sum = 0;
            foreach (Ticket ticket in Tickets) 
            {
                sum += ticket.Tariff.Price;
            }
            return sum;
        }
        public override string ToString()
        {
            return $"{Name} [{PassportId}] from {Country}";
        }
    }
}
