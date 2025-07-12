using Persistense.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Persistense.Repository
{
    public class EfUnitOfWork : IUnitOfWork
    {
        private readonly AppDbContext _context;
        private readonly Lazy<IRepository<Order>> _orderRepository;
        private readonly Lazy<IRepository<Pizza>> _pizzaRepository;

        public EfUnitOfWork(AppDbContext context)
        {
            _context = context;
            _orderRepository = new Lazy<IRepository<Order>>(() => new EfRepository<Order>(context));
            _pizzaRepository = new Lazy<IRepository<Pizza>>(() => new EfRepository<Pizza>(context));
        }

        public IRepository<Order> OrderRepository => _orderRepository.Value;
        public IRepository<Pizza> PizzaRepository => _pizzaRepository.Value;

        public async Task SaveAllAsync()
        {
            await _context.SaveChangesAsync();
        }

        public async Task DeleteDataBaseAsync()
        {
            await _context.Database.EnsureDeletedAsync();
        }

        public async Task CreateDataBaseAsync()
        {
            await _context.Database.EnsureCreatedAsync();
        }
    }
}
