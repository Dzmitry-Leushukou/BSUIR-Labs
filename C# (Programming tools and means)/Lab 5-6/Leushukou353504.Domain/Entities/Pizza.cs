using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Domain.Entities
{
    public class Pizza : Entity
    {
        public string Name { get; set; }
        public string ImagePath { get; set; }
        public int Time { get; set; }
        public int OrderId { get; set; }
        public Order Order { get; set; }
    }
}
