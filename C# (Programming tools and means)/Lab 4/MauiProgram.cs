using Lab_1.Services;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Http;

namespace Lab_1
{
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

#if DEBUG
    		builder.Logging.AddDebug();
#endif
            builder.Services.AddTransient<IDbService, SQLiteService>();
            builder.Services.AddTransient<Lab3>();

            
            builder.Services.AddHttpClient<IRateService>(opt =>
            {
                opt.BaseAddress = new Uri("https://www.nbrb.by/api/exrates/rates");
            });
            builder.Services.AddTransient<IRateService, RateService>();
            builder.Services.AddTransient<Lab4>();

            return builder.Build();
        }
    }
}
