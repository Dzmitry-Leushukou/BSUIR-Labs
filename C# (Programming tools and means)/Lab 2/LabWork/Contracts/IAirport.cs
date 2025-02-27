using _353504_Levshukov_Lab5.Collections;
using _353504_Levshukov_Lab5.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _353504_Levshukov_Lab5.Contracts
{
    internal interface IAirport
    {
        void CreateNewTariff(string route, double cost);
        void AddPassenger(string name, string country, string id);
        bool SellTicket(string route, string date, string? name = null, string? id = null);
        double GetSumOfTickets(string? name=null,string? id=null);
        MyCustomCollection<Passanger>? GetListOfPeopleByDate(string date);

    }
}
