using MediatR;
using Domain.Abstractions;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1.Commands
{
    public class AddPizzaCommandHandler : IRequestHandler<AddPizzaCommand, Pizza>
    {
        private readonly IUnitOfWork _unitOfWork;

        public AddPizzaCommandHandler(IUnitOfWork unitOfWork)
        {
            _unitOfWork = unitOfWork;
        }

        public async Task<Pizza> Handle(AddPizzaCommand request, CancellationToken cancellationToken)
        {
            var pizza = new Pizza
            {
                Name = request.Name,
                ImagePath = request.ImagePath,
                Time = request.Time,
                OrderId = request.OrderId
            };
            await _unitOfWork.PizzaRepository.AddAsync(pizza, cancellationToken);
            await _unitOfWork.SaveAllAsync();
            return pizza;
        }
    }
}
