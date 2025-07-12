using Lab_5_6.ViewModels;
namespace Lab_5_6.Pages;

public partial class PizzaDetails : ContentPage
{
	public PizzaDetails(PizzaDetailsViewModel viewModel)
    {
		InitializeComponent();
        BindingContext = viewModel;
    }
}