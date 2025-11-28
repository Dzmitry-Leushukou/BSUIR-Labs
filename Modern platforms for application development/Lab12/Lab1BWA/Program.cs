using Lab1BWA;
using Lab1BWA.Services;
using Microsoft.AspNetCore.Components.Web;
using Microsoft.AspNetCore.Components.WebAssembly.Hosting;

var builder = WebAssemblyHostBuilder.CreateDefault(args);
builder.RootComponents.Add<App>("#app");
builder.RootComponents.Add<HeadOutlet>("head::after");

// Configure HttpClient with API base address from configuration
var apiBaseUrl = builder.Configuration["ApiSettings:ApiBaseUrl"] ?? "http://localhost:5002/api";
Console.WriteLine($"[WebAssembly] Configuring HttpClient with BaseAddress: {apiBaseUrl}");

builder.Services.AddScoped(sp => 
{
    var httpClient = new HttpClient();
    httpClient.BaseAddress = new Uri(apiBaseUrl);
    return httpClient;
});

// Register DataService with IAccessTokenProvider
builder.Services.AddScoped<IDataService, DataService>();

builder.Services.AddOidcAuthentication(options =>
{
  // Configure your authentication provider options here.
    // For more information, see https://aka.ms/blazor-standalone-auth
  builder.Configuration.Bind("Keycloak", options.ProviderOptions);

    // Ensure required scopes are included
   options.ProviderOptions.DefaultScopes.Add("openid");
    options.ProviderOptions.DefaultScopes.Add("profile");
    options.ProviderOptions.DefaultScopes.Add("email");
});

await builder.Build().RunAsync();
