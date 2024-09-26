using _353504_Levshukov_Lab5.Collections;
using _353504_Levshukov_Lab5.Entities;
using LabWork.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _353504_Levshukov_Lab5
{
    static internal class UI
    {
        static public void TicketSellOperation(bool status)
        {
            if (status)
            {
                //Console.WriteLine("Ticket purchased successfully\n");
                return;
            }

            //Console.WriteLine("An error occurred while purchasing a ticket\n");
        }

        static public void SumOfTicketsOutput(double cost, string? name=null, string? id=null)
        {
            if (cost==-1)
            {
                Console.WriteLine("Passanger not found\n");
                return ;
            }

            Console.Write("Sum of ticket by ");
            if (name != null)
                Console.Write(name+" ");
            if (id != null)
                Console.Write($"[{id}] ");
            Console.Write($"is {cost}$\n\n");
        }

        static public void ShowPassangerByDate(string date,MyCustomCollection<Passanger>? Passangers)
        {
            if(Passangers==null||Passangers.Count==0)
            {
                Console.WriteLine($"Passangers at {date} not found\n");
                return;
            }

            Console.WriteLine($"Passangers at {date}:");
            for(int i=0;i<Passangers.Count;i++) 
            {
                Console.WriteLine($"{Passangers[i].Name} [{Passangers[i].Id}] {Passangers[i].Country}");
            }
            Console.WriteLine("\n");
        }

        static public void ShowJournal(MyCustomCollection<PassangerLog> passangerLogs, MyCustomCollection<TariffLog> tariffLogs)
        {
            Console.WriteLine("===Journal===");

            Console.WriteLine("===Passanger Log===");
            foreach (PassangerLog Pass in passangerLogs)
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
    }
}
