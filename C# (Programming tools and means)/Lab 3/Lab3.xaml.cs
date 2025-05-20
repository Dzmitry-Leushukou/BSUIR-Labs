using System.Collections.ObjectModel;
using Microsoft.Maui.Controls;
using Lab_1.Services;
using Lab_1.Entities;
using System.Diagnostics.Metrics;
using SQLitePCL;

namespace Lab_1;

public partial class Lab3 : ContentPage
{
    private IDbService _dbService;

    public Lab3(IDbService dbService)
	{
		InitializeComponent();
		_dbService = dbService;
        _dbService.Init();
        routePicker.ItemsSource = new ObservableCollection<Route>(_dbService.GetAllRoutes());
    }

    private void OnRouteSelected(object sender, EventArgs e)
    {
        if(routePicker.SelectedItem != null)
        {
            if (routePicker.SelectedItem is Route selectedRoute)
                attractionView.ItemsSource = _dbService.GetRouteAttractions(selectedRoute.Id);
        }
    }
}