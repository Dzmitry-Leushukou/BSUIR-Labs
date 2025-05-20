using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MediatR;
using Application1.Commands;
using Application1.Queries;
namespace Lab_5_6.ViewModels
{
    public partial class AddOrderViewModel : ObservableObject
    {
        private readonly IMediator _mediator;

        public AddOrderViewModel(IMediator mediator)
        {
            _mediator = mediator;
        }

        [ObservableProperty]
        private string name;

        [RelayCommand]
        private async Task SaveOrder()
        {
            var name = string.IsNullOrWhiteSpace(Name) ? " " : Name.Trim();

            var command = new AddOrderCommand { Name = name };
            await _mediator.Send(command);
            var superheroes = await _mediator.Send(new GetAllOrdersQuery());
            await Shell.Current.GoToAsync("..");
        }
    }
}
