using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class PassengerLog : AirportLog
    {
        private Passenger Passenger { get; set; }
        public PassengerLog(Passenger passanger, DateTime date, string info = "") : base(date, info)
        {
            Passenger = passanger;
        }

        public override string ToString()
        {
            return $"[{Date}] New passenger: {Passenger.ToString()}";
        }
    }
}
