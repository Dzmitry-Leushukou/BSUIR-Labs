using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Lab_5_6.Pages;
using Lab_5_6.ViewModels;

namespace Lab_5_6
{
    public static class DependencyInjection
    {
        public static IServiceCollection RegisterPages(this IServiceCollection services)
        {
            services
                .AddTransient<OrdersPage>()
                .AddTransient<PizzaDetails>()
                .AddTransient<AddOrderPage>()
                .AddTransient<AddPizzaPage>();
            return services;
        }

        public static IServiceCollection RegisterViewModels(this IServiceCollection services)
        {
            services
                .AddTransient<OrderViewModel>()
                .AddTransient<PizzaDetailsViewModel>()
                .AddTransient<AddOrderViewModel>()
                .AddTransient<AddPizzaViewModel>();
            return services;
        }
    }
}
