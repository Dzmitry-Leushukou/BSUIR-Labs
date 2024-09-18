using _353504_Levshukov_Lab5.Collections;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _353504_Levshukov_Lab5.Entities
{
    internal class Passanger
    {
        public string Name { get; private set; }
        public string Id { get; private set; }
        public string Country { get; private set; }

        public MyCustomCollection<Ticket> Tickets { get; private set; }

        public Passanger(string name, string id, string country)
        {
            Name = name;
            Id = id;
            Country = country;
            Tickets = new MyCustomCollection<Ticket>();
        }

    }
}
