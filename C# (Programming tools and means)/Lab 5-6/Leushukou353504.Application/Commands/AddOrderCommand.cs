using MediatR;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1.Commands
{
    public class AddOrderCommand : IRequest<Order>
    {
        public string Name { get; set; }
    }
}
