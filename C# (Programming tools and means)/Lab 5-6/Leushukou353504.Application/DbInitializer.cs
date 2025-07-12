using Microsoft.Extensions.DependencyInjection;
using Domain.Abstractions;
using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Application1
{
    public static class DbInitializer
    {
        public static async Task Initialize(IServiceProvider services)
        {
            var unitOfWork = services.GetRequiredService<IUnitOfWork>();

            await unitOfWork.DeleteDataBaseAsync();
            await unitOfWork.CreateDataBaseAsync();

            var order1 = new Order { Name = "Order for Anton"};
            var order2 = new Order { Name = "Order for Alex"};
            var order3 = new Order { Name = "Order for Dzmitry"};
            var order4 = new Order { Name = "Order for Egor" };
            var order5 = new Order { Name = "Order for Kate" };


            await unitOfWork.OrderRepository.AddAsync(order1);
            await unitOfWork.OrderRepository.AddAsync(order2);
            await unitOfWork.OrderRepository.AddAsync(order3);
            await unitOfWork.OrderRepository.AddAsync(order4);
            await unitOfWork.OrderRepository.AddAsync(order5);
            await unitOfWork.SaveAllAsync();

            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Margarita",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Margarita.png",
                Time = 15,
                OrderId = order1.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Peperoni",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Peperoni.jpg",
                Time = 18,
                OrderId = order2.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "4 cheese",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\4cheese.jfif",
                Time =  20,
                OrderId = order2.Id
            }); 
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Pala",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Pala.png",
                Time = 20,
                OrderId = order3.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Margarita",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Margarita.png",
                Time = 15,
                OrderId = order3.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Japanian",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Japanian.webp",
                Time = 35,
                OrderId = order3.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Peperoni",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Peperoni.jpg",
                Time = 18,
                OrderId = order4.Id
            });
            await unitOfWork.PizzaRepository.AddAsync(new Pizza
            {
                Name = "Pala",
                ImagePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\C# (Programming tools and means)\\Lab 5-6\\Resources\\Pala.png",
                Time = 20,
                OrderId = order5.Id
            });

            await unitOfWork.SaveAllAsync();
        }

    }
}
