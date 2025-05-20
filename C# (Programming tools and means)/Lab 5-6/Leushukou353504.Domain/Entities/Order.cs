using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Domain.Entities
{
    public class Order : Entity
    {
        public string Name { get; set; }
        public int Time
        { 
            get 
            { 
                int mx = 0; 
                foreach(var p in Pizzas) 
                    if (p.Time > mx) 
                        mx = p.Time;
                return mx; 
            }
        }
        public List<Pizza> Pizzas { get; set; } = new List<Pizza>();
    }
}
