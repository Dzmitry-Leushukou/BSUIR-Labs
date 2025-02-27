using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class Journal
    {
        public delegate void AddPassengerDelegate(Passenger p);
        public AddPassengerDelegate AddPassenger;
        public delegate void AddTariffDelegate(Tariff t);
        public AddTariffDelegate AddTariff;

        private List<PassengerLog> passengerLogs { get; set; }
        private List<TariffLog> tariffLogs { get; set; }

        public Journal()
        {
            passengerLogs = new List<PassengerLog>();
            tariffLogs = new List<TariffLog>();

            AddPassenger += AddPassengerEvent;
            AddTariff += AddTariffEvent;
        }

        public void LogEvent(string type, Passenger? passanger = null, Tariff? tariff = null)
        {
            if (type == "AddPassengerEvent")
            {
                AddPassengerEvent(passanger);
                return;
            }
            AddTariffEvent(tariff);
        }

        public void AddPassengerEvent(Passenger passanger)
        {
            passengerLogs.Add(new PassengerLog(passanger, DateTime.Now));
        }

        public void AddTariffEvent(Tariff tariff)
        {
            tariffLogs.Add(new TariffLog(tariff, DateTime.Now));
        }

        public void ShowJournal()
        {
            UI.ShowJournal(passengerLogs, tariffLogs);
        }
    }
}
