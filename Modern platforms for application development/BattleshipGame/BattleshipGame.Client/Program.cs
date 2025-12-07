using Microsoft.AspNetCore.Components.Web;
using Microsoft.AspNetCore.Components.WebAssembly.Hosting;
using BattleshipGame.Client;
using BattleshipGame.Client.Services;

var builder = WebAssemblyHostBuilder.CreateDefault(args);
builder.RootComponents.Add<App>("#app");
builder.RootComponents.Add<HeadOutlet>("head::after");

// Регистрация сервисов
builder.Services.AddScoped<GameHubService>();
builder.Services.AddScoped<GameStateService>();

// HttpClient
builder.Services.AddScoped(sp => new HttpClient
{
    BaseAddress = new Uri(builder.HostEnvironment.BaseAddress)
});

// Регистрируем ILogger
builder.Services.AddLogging();

await builder.Build().RunAsync();