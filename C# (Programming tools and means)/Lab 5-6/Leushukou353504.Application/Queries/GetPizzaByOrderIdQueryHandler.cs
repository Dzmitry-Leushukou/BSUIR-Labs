using MediatR;
using Domain.Abstractions;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1.Queries
{
    public class GetPizzaByOrderIdQueryHandler : IRequestHandler<GetPizzaByOrderIdQuery, List<Pizza>>
    {
        private readonly IUnitOfWork _unitOfWork;

        public GetPizzaByOrderIdQueryHandler(IUnitOfWork unitOfWork)
        {
            _unitOfWork = unitOfWork;
        }

        public async Task<List<Pizza>> Handle(GetPizzaByOrderIdQuery request, CancellationToken cancellationToken)
        {
            return (await _unitOfWork.PizzaRepository.ListAsync(a => a.OrderId == request.OrderId, cancellationToken)).ToList();
        }
    }
}
