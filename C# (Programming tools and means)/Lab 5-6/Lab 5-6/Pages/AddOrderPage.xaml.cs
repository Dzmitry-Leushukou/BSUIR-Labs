namespace Lab_5_6.Pages;
using Lab_5_6.ViewModels;
public partial class AddOrderPage : ContentPage
{
	public AddOrderPage(AddOrderViewModel viewModel)
	{
		InitializeComponent();
		BindingContext = viewModel;
	}
}