using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class Airport : IAirport
    {
        private Dictionary<string, Tariff> Tariffs = new();
        private List<Passenger> Passengers = new();

        public delegate void AddPassengerDelegate(Passenger passanger);
        public event AddPassengerDelegate? AddPassengerEvent;

        public delegate void AddTariffDelegate(Tariff tariff);
        public event AddTariffDelegate AddTariffEvent;

        public delegate void RegisterTicketDelegate(Ticket ticket);
        public event RegisterTicketDelegate? RegisterTicketEvent;

        public void CreateNewTariff(string route, double cost)
        {
            Tariffs.Add(route, new Tariff(route, cost));
            AddTariffEvent.Invoke(new Tariff(route, cost));
        }
        public void AddPassenger(string name, string country, string id)
        {
            Passengers.Add(new Passenger(name, id, country));
            AddPassengerEvent.Invoke(new Passenger(name, id, country));
        }
        public bool SellTicket(string route, string date, string? name = null, string? id = null)
        {
            if (name == null && id == null)
                return false;

            int pindex = FindPassenger(name, id);

            if (pindex == -1|| Tariffs[route]==null)
                return false;

            Ticket ticket = new(Tariffs[route], date, Passengers[pindex]);
            Passengers[pindex].Tickets.Add(ticket);
            RegisterTicketEvent.Invoke(ticket);
            return true;
        }
        public double GetSumOfTickets(string? name = null, string? id = null)
        {
            int pindex = FindPassenger(name, id);

            if (pindex == -1) return -1;

            double sum = 0;

            for (int i = 0; i < Passengers[pindex].Tickets.Count; i++)
            {
                sum += Passengers[pindex].Tickets[i].Tariff.Price;
            }

            return sum;
        }
        public List<Passenger>? GetListOfPeopleByDate(string date)
        {
            List<Passenger> found = new();

            for (int i = 0; i < Passengers.Count; i++)
            {

                for (int j = 0; j < Passengers[i].Tickets.Count; j++)
                    if (Passengers[i].Tickets[j].Date == date)
                    {
                        found.Add(Passengers[i]);
                        break;
                    }
            }
            return found;
        }

        private int FindPassenger(string? name = null, string? id = null)
        {
            int res = -1;
            for (int i = 0; i < Passengers.Count; i++)
            {
                if (((Passengers[i].Name == name && name != null) && (id != null && Passengers[i].PassportId == id))
                || ((Passengers[i].Name == name && name != null) && (id == null))
                || ((name == null) && (Passengers[i].PassportId == id && id != null)))
                {
                    res = i;
                    break;
                }
            }
            return res;
        }
        public void ShowTariffs()
        {
            Tariffs = Tariffs.OrderBy(x => x.Value.Price).ToDictionary(x => x.Key, x => x.Value);
            UI.GetAllTariffs(Tariffs);
        }
        public void GetProfit()
        {
            double sum = 0;
            int count_of_tickets=0;
            foreach(var p in Passengers)
            {
                sum += p.SumOfTickets();
                count_of_tickets += p.Tickets.Count;
            }
            UI.ShowProfit(sum,Passengers.Count, count_of_tickets);
        }
        public void MostImportantPassanger()
        {
            double money = 0;
            Passenger mip=null;

            foreach(var p in Passengers)
            {
                if(p.SumOfTickets()>money)
                {
                    mip = p;
                    money= p.SumOfTickets();
                }    
            }

            UI.MostImportantPasseenger(mip);
        }
        public void SpentMoreThan(double delta)
        {
            int count = Passengers.Aggregate(0, (c, p) => p.SumOfTickets() > delta ? c+1 : c);
            UI.SpentMoreThan(count,delta);
        }
        public void SpentByRoutes(string? name = null, string? id = null)
        {
            int index=FindPassenger(name, id);
            if (index == -1)
                return;
            List<Ticket> Grouped = Passengers[index].Tickets
            .GroupBy(t => new { t.Tariff.Route, t.Date, t.owner })
            .Select(g => new Ticket(new Tariff(g.Key.Route, g.Sum(t => t.Tariff.Price)), g.Key.Date, g.Key.owner)).ToList();
            UI.SpentByRoutes(Grouped);
        }
    }
}
