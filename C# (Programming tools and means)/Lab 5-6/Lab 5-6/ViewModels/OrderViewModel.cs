using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MediatR;
using Application1.Queries;
using Domain.Entities;
using Lab_5_6.Pages;
using System.Collections.ObjectModel;
namespace Lab_5_6.ViewModels
{
    public partial class OrderViewModel : ObservableObject
    {
        private readonly IMediator _mediator;

        public OrderViewModel(IMediator mediator)
        {
            _mediator = mediator;
        }

        public ObservableCollection<Order> Orders { get; set; } = new();
        public ObservableCollection<Pizza> Pizzas { get; set; } = new();

        private Order _selectedOrder;
        public Order SelectedOrder
        {
            get => _selectedOrder;
            set
            {
                SetProperty(ref _selectedOrder, value);
                OnPropertyChanged(nameof(IsOrderSelected));
            }
        }

        public bool IsOrderSelected => SelectedOrder != null;

        [RelayCommand]
        private async Task UpdateOrders()
        {
            var orders = await _mediator.Send(new GetAllOrdersQuery());
            Orders.Clear();
            foreach (var order in orders)
            {
                Orders.Add(order);
            }
        }

        [RelayCommand]
        private async Task UpdatePizzas()
        {
            if (SelectedOrder != null)
            {
                var pizzas = await _mediator.Send(new GetPizzaByOrderIdQuery { OrderId = SelectedOrder.Id });
                Pizzas.Clear();
                foreach (var pizza in pizzas)
                {
                    Pizzas.Add(pizza);
                }
            }
        }

        [RelayCommand]
        private async Task AddOrder()
        {
            Pizzas.Clear();
            await Shell.Current.GoToAsync(nameof(AddOrderPage));
        }

        [RelayCommand]
        private async Task AddPizza()
        {
            Pizzas.Clear();
            if (SelectedOrder != null)
            {
                await Shell.Current.GoToAsync(nameof(AddPizzaPage), new Dictionary<string, object> { { "OrderId", SelectedOrder.Id } });
            }
        }

        [RelayCommand]
        private async Task ShowDetails(Pizza pizza)
        {
            Pizzas.Clear();
            await Shell.Current.GoToAsync(nameof(PizzaDetails), new Dictionary<string, object> { { "Pizza", pizza } });
        }
    }
}
