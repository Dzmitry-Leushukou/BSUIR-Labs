using Microsoft.AspNetCore.Components;
using Microsoft.AspNetCore.Components.WebAssembly.Hosting;
using Microsoft.AspNetCore.Components.WebAssembly.Authentication;
using Microsoft.Extensions.Http;
using Lab1.BlazorWasm;
using Lab1.BlazorWasm.Services;

var builder = WebAssemblyHostBuilder.CreateDefault(args);
builder.RootComponents.Add<App>("#app");

// Загрузка конфигурации
var apiBaseUrl = builder.Configuration["ApiBaseUrl"] ?? "https://localhost:7070";

// Регистрация HttpClient с базовым адресом API
builder.Services.AddHttpClient("Lab1.BlazorWasm.ServerAPI", client =>
    client.BaseAddress = new Uri(apiBaseUrl))
.AddHttpMessageHandler<BaseAddressAuthorizationMessageHandler>();

// Создание HttpClient для DataService с добавлением токена
builder.Services.AddScoped(sp => 
{
    var factory = sp.GetRequiredService<IHttpClientFactory>();
    return factory.CreateClient("Lab1.BlazorWasm.ServerAPI");
});

// Регистрация DataService как Scoped сервиса
builder.Services.AddScoped<IDataService, DataService>();

// Добавление аутентификации с Keycloak
builder.Services.AddOidcAuthentication(options =>
{
    // Конфигурация провайдера аутентификации
    builder.Configuration.Bind("Keycloak", options.ProviderOptions);
    
    // Обязательно включаем параметры для OIDC
    options.ProviderOptions.DefaultScopes.Add("openid");
    options.ProviderOptions.DefaultScopes.Add("profile");
    options.ProviderOptions.DefaultScopes.Add("email");
});

await builder.Build().RunAsync();
