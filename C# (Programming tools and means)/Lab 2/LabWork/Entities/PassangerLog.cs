using _353504_Levshukov_Lab5.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LabWork.Entities
{
    internal class PassangerLog:AirportLog
    {
        private Passanger Passanger { get; set; }
        public PassangerLog(Passanger passanger, DateTime date, string info=""):base(date,info)
        {
            Passanger = passanger;
        }

        public override string ToString()
        {
            return $"[{Date}] New Passanger ({Passanger.Name}) [ID: {Passanger.Id}] from {Passanger.Country}";
        }
    }
}
