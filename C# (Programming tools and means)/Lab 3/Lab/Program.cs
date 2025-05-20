using Lab.Entities;
using System;

internal class Program
{
    private static void Main(string[] args)
    {
        Airport airport = new Airport();
        Journal journal = new Journal();
        airport.AddTariffEvent += journal.AddTariffEvent;
        airport.AddPassengerEvent += journal.AddPassengerEvent;
        airport.RegisterTicketEvent += (Ticket ticket) =>
        {
            UI.RegisterTicketEvent(ticket);
        };

        airport.AddPassenger("Ivanov Ivan", "Belarus", "12345678");
        airport.AddPassenger("Qwerty Anton", "China", "683904");
        airport.AddPassenger("Faddad Udjsd", "Poland", "8877443");
        airport.AddPassenger("Dragin Ksts", "Litva", "1290657");
        airport.AddPassenger("Lagun Jan", "Latvia", "348907");

        airport.CreateNewTariff("Minsk-Dublin", 120.50);
        airport.CreateNewTariff("Dublin-Riga", 420);
        airport.CreateNewTariff("Minsk-Riga", 20.40);
        airport.CreateNewTariff("Riga-Minsk", 20);

        airport.SellTicket("Minsk-Dublin", "12.02.2020", "Ivanov Ivan");
        airport.SellTicket("Dublin-Riga", "13.02.2020", null, "12345678");
        airport.SellTicket("Dublin-Riga", "13.02.2020", "Faddad Udjsd");
        airport.SellTicket("Dublin-Riga", "13.02.2020", "Qwerty Anton");
        airport.SellTicket("Minsk-Riga", "13.02.2020", null, "1290657");
        airport.SellTicket("Riga-Minsk", "12.02.2020", "Lagun Jan");

        UI.SumOfTicketsOutput(airport.GetSumOfTickets("Ivanov Ivan"), "Ivanov Ivan");
        UI.SumOfTicketsOutput(airport.GetSumOfTickets(null, "683904"), null, "683904");

        UI.ShowPassangerByDate("13.02.2020", airport.GetListOfPeopleByDate("13.02.2020"));

        journal.ShowJournal();

        airport.ShowTariffs();
        airport.GetProfit();
        airport.MostImportantPassanger();
        airport.SpentMoreThan(100);
        airport.SpentByRoutes("Ivanov Ivan",null);
        airport.SellTicket("Minsk-Dublin", "12.02.2020", "Ivanov Ivan");
        airport.SpentByRoutes("Ivanov Ivan", null);
    }
}