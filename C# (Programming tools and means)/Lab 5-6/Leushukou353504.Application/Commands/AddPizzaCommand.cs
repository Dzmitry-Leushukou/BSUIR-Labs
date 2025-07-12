using MediatR;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1.Commands
{
    public class AddPizzaCommand : IRequest<Pizza>
    {
        public string Name { get; set; }
        public string ImagePath { get; set; }
        public int Time { get; set; }
        public int OrderId { get; set; }
    }
}
