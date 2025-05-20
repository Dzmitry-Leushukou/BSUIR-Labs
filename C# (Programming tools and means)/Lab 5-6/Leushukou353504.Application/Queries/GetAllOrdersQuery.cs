using MediatR;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1.Queries
{
    public class GetAllOrdersQuery : IRequest<List<Order>>
    {
    }
    public class GetPizzaByOrderIdQuery : IRequest<List<Pizza>>
    {
        public int OrderId { get; set; }
    }
}
