namespace Lab_5_6.Pages;
using Lab_5_6.ViewModels;
public partial class OrdersPage : ContentPage
{
    private readonly OrderViewModel _viewModel;
    public OrdersPage(OrderViewModel viewModel)
    {
        InitializeComponent();
        _viewModel = viewModel;
        BindingContext = viewModel;
        // Загрузка данных
        viewModel.UpdateOrdersCommand.Execute(null);
    }

    private async void OnOrderSelected(object sender, EventArgs e)
    {
        var vm = (OrderViewModel)BindingContext;
        await vm.UpdatePizzasCommand.ExecuteAsync(null);
    }
    protected override async void OnAppearing()
    {
        base.OnAppearing();
        await _viewModel.UpdateOrdersCommand.ExecuteAsync(null);
    }
}