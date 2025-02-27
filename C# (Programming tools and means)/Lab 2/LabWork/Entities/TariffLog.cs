using _353504_Levshukov_Lab5.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LabWork.Entities
{
    internal class TariffLog:AirportLog
    {
        private Ticket Tariff { get;set; }
        public TariffLog(Ticket tarrif,DateTime date,string info=""):base(date,info)
        {
            Tariff = tarrif;
        }

        public override string ToString()
        {
            return $"[{Date}] New tariff ({Tariff.Route}) with price: {Tariff.Price}$";
        }
    }
}
