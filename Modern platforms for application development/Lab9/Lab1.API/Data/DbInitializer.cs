using Lab1.Domain.Entities;
using Microsoft.EntityFrameworkCore;

namespace Lab1.API.Data;

public static class DbInitializer
{
    public static async Task SeedData(WebApplication app)
    {
        using var scope = app.Services.CreateScope();
        var context = scope.ServiceProvider.GetRequiredService<AppDbContext>();

        await context.Database.MigrateAsync();

        if (await context.Cars.AnyAsync())
            return;

        // Убедимся, что wwwroot/Images существует
        var env = scope.ServiceProvider.GetRequiredService<IWebHostEnvironment>();
        var webRoot = env.WebRootPath ?? Path.Combine(Directory.GetCurrentDirectory(), "wwwroot");
        var imagesDir = Path.Combine(webRoot, "Images");
        Directory.CreateDirectory(imagesDir);

        // Хелпер: если файла нет — ставим noimage.jpg
        string ImgOrDefault(string? relative)
        {
            if (string.IsNullOrWhiteSpace(relative))
                return "Images/noimage.jpg";

            var rel = relative.Replace("\\", "/").TrimStart('/');
            if (!rel.StartsWith("Images/", StringComparison.OrdinalIgnoreCase))
                rel = $"Images/{rel}";

            var full = Path.Combine(webRoot, rel.Replace("/", Path.DirectorySeparatorChar.ToString()));

            return File.Exists(full) ? rel : "Images/noimage.jpg";
        }

        var categories = new List<Category>
        {
            new Category {Id=1, Name="Седаны",      NormalizedName="sedans"},
            new Category {Id=2, Name="Купе",        NormalizedName="coupe"},
            new Category {Id=3, Name="Универсалы",  NormalizedName="universals"},
            new Category {Id=4, Name="Хетчбэки",    NormalizedName="hatchbacks"},
            new Category {Id=5, Name="Минивэны",    NormalizedName="minivans"},
            new Category {Id=6, Name="Родстеры",    NormalizedName="roadsters"}
        };

        await context.Categories.AddRangeAsync(categories);
        await context.SaveChangesAsync();

        var cars = new List<Car>
        {
            new Car {
                Id = 1, Name = "BMW E34", Description = "Легендарный немецкий седан", Price = 15000,
                ImagePath = ImgOrDefault("Images/bmw_e34.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="sedans")
            },
            new Car {
                Id = 2, Name = "Mercedes W124", Description = "Надежный немецкий седан", Price = 12000,
                ImagePath = ImgOrDefault("Images/mercedes_w124.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="sedans")
            },
            new Car {
                Id = 3, Name = "Audi TT", Description = "Стильное купе", Price = 25000,
                ImagePath = ImgOrDefault("Images/audi_tt.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="coupe")
            },
            new Car {
                Id = 4, Name = "BMW M4", Description = "Спортивное купе", Price = 65000,
                ImagePath = ImgOrDefault("Images/bmw_m4.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="coupe")
            },
            new Car {
                Id = 5, Name = "Volvo V90", Description = "Практичный универсал", Price = 45000,
                ImagePath = ImgOrDefault("Images/volvo_v90.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="universals")
            },
            new Car {
                Id = 6, Name = "Audi A6 Avant", Description = "Премиальный универсал", Price = 52000,
                ImagePath = ImgOrDefault("Images/audi_a6_avant.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="universals")
            },
            new Car {
                Id = 7, Name = "Volkswagen Golf", Description = "Классический хетчбэк", Price = 22000,
                ImagePath = ImgOrDefault("Images/vw_golf.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="hatchbacks")
            },
            new Car {
                Id = 8, Name = "Ford Focus", Description = "Популярный хетчбэк", Price = 19000,
                ImagePath = ImgOrDefault("Images/ford_focus.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="hatchbacks")
            },
            new Car {
                Id = 9, Name = "Toyota Sienna", Description = "Семейный минивэн", Price = 32000,
                ImagePath = ImgOrDefault("Images/toyota_sienna.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="minivans")
            },
            new Car {
                Id = 10, Name = "Honda Odyssey", Description = "Комфортный минивэн", Price = 35000,
                ImagePath = ImgOrDefault("Images/honda_odyssey.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="minivans")
            },
            new Car {
                Id = 11, Name = "Mazda MX-5", Description = "Легендарный родстер", Price = 28000,
                ImagePath = ImgOrDefault("Images/mazda_mx5.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="roadsters")
            },
            new Car {
                Id = 12, Name = "Porsche 718 Boxster", Description = "Спортивный родстер", Price = 65000,
                ImagePath = ImgOrDefault("Images/porsche_boxster.png"), Mime = "image/png",
                Category = categories.First(c => c.NormalizedName=="roadsters")
            }
        };

        await context.Cars.AddRangeAsync(cars);
        await context.SaveChangesAsync();
    }
}
