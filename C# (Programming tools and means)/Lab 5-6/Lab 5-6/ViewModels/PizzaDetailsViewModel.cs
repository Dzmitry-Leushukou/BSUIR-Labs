using Domain.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;

namespace Lab_5_6.ViewModels
{
    [QueryProperty(nameof(Pizza),"Pizza")]
    public partial class PizzaDetailsViewModel:ObservableObject
    {
        [ObservableProperty]
        private Pizza pizza;
    }
}
