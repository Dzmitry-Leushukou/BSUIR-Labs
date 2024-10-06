using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Lab.Entities;

static internal class UI
{
    static public void SumOfTicketsOutput(double cost, string? name=null, string? id=null)
    {
        if (cost==-1)
        {
            Console.WriteLine("Passenger not found\n");
            return ;
        }

        Console.Write("Sum of ticket by ");
        if (name != null)
            Console.Write(name+" ");
        if (id != null)
            Console.Write($"[{id}] ");
        Console.Write($"is {cost}$\n\n");
    }
    static public void ShowPassangerByDate(string date,List<Passenger>? Passangers)
    {
        if(Passangers==null||Passangers.Count==0)
        {
            Console.WriteLine($"Passangers at {date} not found\n");
            return;
        }

        Console.WriteLine($"Passangers at {date}:");
        for(int i=0;i<Passangers.Count;i++) 
        {
            Console.WriteLine($"{Passangers[i].Name} [{Passangers[i].PassportId}] {Passangers[i].Country}");
        }
        Console.WriteLine("\n");
    }
    static public void ShowJournal(List<PassengerLog> passangerLogs, List<TariffLog> tariffLogs)
    {
        Console.WriteLine("===Journal===");

        Console.WriteLine("===Passanger Log===");
        foreach (PassengerLog Pass in passangerLogs)
        {
            Console.WriteLine(Pass.ToString());
        }

        Console.WriteLine("\n===Tariff Log===");
        foreach (TariffLog tariff in tariffLogs)
        {
            Console.WriteLine(tariff.ToString());
        }

        Console.WriteLine("\n");
    }
    static public void RegisterTicketEvent(Ticket ticket)
    {
        Console.WriteLine($"Register new ticket: {ticket.ToString()}\n"); 
    }
    static public void GetAllTariffs(Dictionary<string,Tariff>tariffs)
    {
        foreach(KeyValuePair<string, Tariff> tariff in tariffs)
        {
            Console.WriteLine($"{tariff.Key}  {tariff.Value.Price}");
        }
    Console.WriteLine();
}
    static public void ShowProfit(double sum, int pcount,int tcount)
    {
        Console.WriteLine($"Profit of airport by {pcount} passangers and {tcount} selled ticket is {sum}\n");
    }
    static public void MostImportantPasseenger(Passenger p)
    {
    Console.WriteLine($"The most impotrant passanger is {p.ToString()}. He/She spent {p.SumOfTickets()}\n");
    }
    static public void SpentMoreThan(int count,double delta)
    {
    Console.WriteLine($"{count} passangers that spent more than {delta}\n");
    }
    static public void SpentByRoutes(List<Ticket>tickets)
    {
        Console.WriteLine(tickets[0].owner.ToString());
        foreach(Ticket t in tickets)
        {
            Console.WriteLine($"{t.Tariff.Route} {t.Tariff.Price}");
        }
        Console.WriteLine();
    }
}

