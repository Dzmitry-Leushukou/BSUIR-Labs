using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Configuration;

namespace Lab1.Extensions;

public static class HostingExtensions
{
    /// <summary>
    /// Регистрируем «памятные» реализации сервисов из ранних лаб,
    /// чтобы проект запускался до подключения API.
    /// Также регистрируем конфигурацию Keycloak.
    /// </summary>
    public static WebApplicationBuilder RegisterCustomServices(this WebApplicationBuilder builder)
    {
        builder.Services.AddSingleton<ICategoryService, MemoryCategoryService>();
        builder.Services.AddSingleton<ICarService, MemoryCarService>();

        // Регистрируем секцию Keycloak в конфигурацию (опция для DI)
        builder.Services.Configure<KeycloakData>(builder.Configuration.GetSection("Keycloak"));

        return builder;
    }
}
