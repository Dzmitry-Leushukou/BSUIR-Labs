using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab5.Domain.Entitites
{
    public class Driver:IEquatable<Driver>
    {
        public string Name { get;  set; }
        public string Id { get;  set; }
        public Bus Bus { get;  set; }
        public Driver(string name, string id, Bus bus)
        {
            Name = name;
            Id = id;
            Bus = bus;
        }
        public Driver() { }
        public bool Equals(Driver? obj)
        {
            return Name==obj.Name&& Id == obj.Id && Bus.Id == obj.Bus.Id && Bus.Capacity == obj.Bus.Capacity && Bus.Id == obj.Bus.Id;
        }
    }
}
