using Lab_5_6.Pages;
namespace Lab_5_6
{
    public partial class AppShell : Shell
    {
        public AppShell()
        {
            InitializeComponent();
            Routing.RegisterRoute(nameof(OrdersPage), typeof(OrdersPage));
            Routing.RegisterRoute(nameof(PizzaDetails), typeof(PizzaDetails));
            Routing.RegisterRoute(nameof(AddOrderPage), typeof(AddOrderPage));
            Routing.RegisterRoute(nameof(AddPizzaPage), typeof(AddPizzaPage));
        }
    }
}
