using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab5.Domain.Entitites
{
    public class Bus
    {
        public string Id { get;  set; }
        public string Model { get;  set; }
        public int Capacity { get;  set; }
        public Bus(string id, string model, int capacity) 
        {
            Id = id;
            Model = model;
            Capacity = capacity;
        }
        public Bus() { }
    }
}
