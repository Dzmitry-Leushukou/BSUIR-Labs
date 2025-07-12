using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MediatR;
using Application1.Commands;

namespace Lab_5_6.ViewModels
{
    public partial class AddPizzaViewModel : ObservableObject
    {
        private readonly IMediator _mediator;

        public AddPizzaViewModel(IMediator mediator)
        {
            _mediator = mediator;
        }

        [ObservableProperty]
        private string name;

        [ObservableProperty]
        private string imagePath;

        [ObservableProperty]
        private int time;

        [ObservableProperty]
        private int orderId;

        [RelayCommand]
        private async Task SavePizza()
        {
            var abilityName = string.IsNullOrWhiteSpace(Name) ? " " : Name.Trim();
            var imagePathValue = ImagePath ?? "";

            var command = new AddPizzaCommand
            {
                Name = abilityName,
                ImagePath = imagePathValue,
                Time = time,
                OrderId = orderId
            };
            await _mediator.Send(command);
            await Shell.Current.GoToAsync("..");
        }
    }
}
