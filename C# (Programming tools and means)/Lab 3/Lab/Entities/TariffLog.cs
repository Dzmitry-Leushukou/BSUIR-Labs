using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class TariffLog : AirportLog
    {
        private Tariff Tariff { get; set; }
        public TariffLog(Tariff tarrif, DateTime date, string info = "") : base(date, info)
        {
            Tariff = tarrif;
        }

        public override string ToString()
        {
            return $"[{Date}] New tariff ({Tariff.Route}) with price: {Tariff.Price}$";
        }
    }
}
