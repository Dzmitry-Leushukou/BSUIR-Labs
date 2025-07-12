using Lab_5_6.ViewModels;
using MediatR;
using Microsoft.Maui.Controls;


namespace Lab_5_6.Pages
{
    [QueryProperty(nameof(OrderId), "OrderId")]
    public partial class AddPizzaPage : ContentPage
    {
        public AddPizzaPage(IMediator mediator)
        {
            InitializeComponent();
            BindingContext = new AddPizzaViewModel(mediator);
        }

        // Property to receive the OrderId from navigation query parameters
        public int OrderId
        {
            set
            {
                if (BindingContext is AddPizzaViewModel viewModel)
                {
                    viewModel.OrderId = value;
                }
            }
        }
    }
}