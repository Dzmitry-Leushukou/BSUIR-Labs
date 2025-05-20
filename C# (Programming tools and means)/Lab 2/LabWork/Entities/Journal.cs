using _353504_Levshukov_Lab5;
using _353504_Levshukov_Lab5.Collections;
using _353504_Levshukov_Lab5.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LabWork.Entities
{
    internal class Journal
    {
        public delegate void AddPassangerDelegate(Passanger p);
        public AddPassangerDelegate? AddPassanger;
        public delegate void AddTariffDelegate(Ticket tariff);
        public AddTariffDelegate? AddTariff;

        private MyCustomCollection<PassangerLog> PassangerAddingEvent { get; set; }
        private MyCustomCollection<TariffLog> TariffAddingEvent { get; set; }

        public Journal() 
        {
            PassangerAddingEvent = new MyCustomCollection<PassangerLog>();
            TariffAddingEvent = new MyCustomCollection<TariffLog>();

            AddPassanger += AddPassangerEvent;
            AddTariff += AddTariffEvent;
        }

        public void LogEvent(string type, Passanger? passanger=null, Ticket? tariff=null)
        {
            if (type == "AddPassangerEvent")
            {
                AddPassangerEvent(passanger);
                return;
            }
            AddTariffEvent(tariff);
        }


        public void AddPassangerEvent(Passanger passanger)
        {
            PassangerAddingEvent.Add(new PassangerLog(passanger, DateTime.Now));
        }

        public void AddTariffEvent(Ticket tariff)
        {
            TariffAddingEvent.Add(new TariffLog(tariff, DateTime.Now));
        }
        
        public void ShowJournal()
        {
            UI.ShowJournal(PassangerAddingEvent, TariffAddingEvent);
        }
    }
}
