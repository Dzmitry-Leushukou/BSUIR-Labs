﻿using _353504_Levshukov_Lab5.Contracts;
using _353504_Levshukov_Lab5.Collections;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;

namespace _353504_Levshukov_Lab5.Entities
{
    internal class Airport : IAirport
    {
        private MyCustomCollection<Ticket> _Tarrifs=new();
        private MyCustomCollection<Passanger> _Passangers = new();
        public void CreateNewTariff(string route, double cost)
        {
            _Tarrifs.Add(new Ticket(route, cost));
        }
        public void AddPassenger(string name, string country, string id)
        {
            _Passangers.Add(new Passanger(name, id, country));
        }
        public bool SellTicket(string route, string date, string? name = null, string? id = null)
        {
            if (name == null && id == null)
                return false;

            int pindex=FindPassanger(name,id), tindex=FindTarrif(route);
            
            if (pindex == -1)
                return false;

            if (tindex == -1)
                return false;

            Ticket ticket = _Tarrifs[tindex];
            ticket.Date = date;
            _Passangers[pindex].Tickets.Add(ticket) ;

            return true;
        }
        public double GetSumOfTickets(string? name = null, string? id = null)
        {
            int pindex = FindPassanger(name, id);

            if (pindex == -1) return -1;

            double sum = 0;

            for(int i = 0; i < _Passangers[pindex].Tickets.Count;i++)
            {
                sum += _Passangers[pindex].Tickets[i].Price;
            }

            return sum;
        }
        public MyCustomCollection<Passanger>? GetListOfPeopleByDate(string date)
        {
            MyCustomCollection<Passanger> found = new MyCustomCollection<Passanger>();

            for (int i = 0; i < _Passangers.Count; i++)
            {

                for (int j = 0; j < _Passangers[i].Tickets.Count; j++)
                    if (_Passangers[i].Tickets[j].Date == date)
                    {
                        found.Add(_Passangers[i]);
                        break;
                    }
            }
            return found;
        }

        private int FindPassanger(string? name = null, string? id = null)
        {
            int res = -1;
            for (int i = 0; i < _Passangers.Count; i++)
            {
                if (((_Passangers[i].Name == name && name != null) && (id != null && _Passangers[i].Id == id))
                || ((_Passangers[i].Name == name && name != null) && (id == null))
                || ((name == null) && (_Passangers[i].Id == id && id != null)))
                {
                    res = i;
                    break;
                }
            }
            return res;
        }

        private int FindTarrif(string route)
        {
            int res = -1;
            for (int i = 0; i < _Tarrifs.Count; i++)
            {
                if (_Tarrifs[i].Route == route)
                {
                    res = i;
                    break;
                }
            }
            return res;
        }
    }
}