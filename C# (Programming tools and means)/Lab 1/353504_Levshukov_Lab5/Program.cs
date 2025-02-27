using _353504_Levshukov_Lab5;
using _353504_Levshukov_Lab5.Entities;
using System;

internal class Program
{
    private static void Main(string[] args)
    {
        Airport airport = new Airport();

        airport.AddPassenger("Ivanov Ivan", "Belarus", "12345678");
        airport.AddPassenger("Qwerty Anton", "China", "683904");
        airport.AddPassenger("Faddad Udjsd", "Poland", "8877443");
        airport.AddPassenger("Dragin Ksts", "Litva", "1290657");
        airport.AddPassenger("Lagun Jan", "Latvia", "348907");

        airport.CreateNewTariff("Minsk-Dublin", 120.50);
        airport.CreateNewTariff("Dublin-Riga", 420);
        airport.CreateNewTariff("Minsk-Riga", 20.40);
        airport.CreateNewTariff("Riga-Minsk", 20);

        UI.TicketSellOperation(airport.SellTicket("Minsk-Dublin", "12.02.2020", "Ivanov Ivan"));
        UI.TicketSellOperation(airport.SellTicket("Dublin-Riga", "13.02.2020", null, "12345678"));
        UI.TicketSellOperation(airport.SellTicket("Dublin-Riga", "13.02.2020", "Faddad Udjsd"));
        UI.TicketSellOperation(airport.SellTicket("Dublin-Riga", "13.02.2020", "Qwerty Anton"));
        UI.TicketSellOperation(airport.SellTicket("Minsk-Riga", "13.02.2020", null, "1290657"));
        UI.TicketSellOperation(airport.SellTicket("Riga-Minsk", "12.02.2020", "Lagun Jan"));

        UI.SumOfTicketsOutput(airport.GetSumOfTickets("Ivanov Ivan"), "Ivanov Ivan");
        UI.SumOfTicketsOutput(airport.GetSumOfTickets(null, "683904"), null, "683904");

        UI.ShowPassangerByDate("13.02.2020", airport.GetListOfPeopleByDate("13.02.2020"));
    }
}