using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System.Reflection;
using Domain.Abstractions;
using Persistense.Data;
using Persistense.Repository;
using Application1.Queries;
using Application1.Commands;
using Application1;
namespace Lab_5_6;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder
            .UseMauiApp<App>()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
            });
        var assembly = Assembly.GetExecutingAssembly();
        using var stream = assembly.GetManifestResourceStream("Lab_5_6.appsettings.json");
        builder.Configuration.AddJsonStream(stream);
        var connStr = builder.Configuration.GetConnectionString("SqliteConnection");
        string dataDirectory = FileSystem.Current.AppDataDirectory + "/";
        connStr = string.Format(connStr, dataDirectory);

        var options = new DbContextOptionsBuilder<AppDbContext>().UseSqlite(connStr).Options;

        builder.Services
            .AddSingleton<AppDbContext>(provider => new AppDbContext(options))
            .AddSingleton<IUnitOfWork, EfUnitOfWork>()
            .AddMediatR(cfg => cfg.RegisterServicesFromAssembly(typeof(GetAllOrdersQuery).Assembly))
            .RegisterPages()
            .RegisterViewModels();

#if DEBUG
        builder.Logging.AddDebug();
#endif

        var app = builder.Build();
        DbInitializer.Initialize(app.Services).Wait();
        return app;
    }
}
