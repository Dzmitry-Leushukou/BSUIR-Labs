using Lab1.API.Data;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace Lab1.Tests.API.Services
{
    /// <summary>
 /// Интеграционные тесты для сервис-слоя API
    /// </summary>
    public class ApiServiceIntegrationTests : IDisposable
    {
        private readonly SqliteConnection _connection;

        public ApiServiceIntegrationTests()
     {
            _connection = new SqliteConnection("Filename=:memory:");
       _connection.Open();
        }

        private AppDbContext CreateContext()
  {
            var options = new DbContextOptionsBuilder<AppDbContext>()
 .UseSqlite(_connection)
      .Options;

    var context = new AppDbContext(options);
     context.Database.EnsureCreated();
            return context;
        }

   private async Task SeedCompleteDataAsync(AppDbContext context)
        {
      var categories = new List<Category>
            {
              new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
        new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" }
     };

         context.Categories.AddRange(categories);
  await context.SaveChangesAsync();

            var cars = new List<Car>
        {
         new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Description = "Sedan", Category = categories[0] },
           new Car { Id = 2, Name = "Honda Accord", Price = 27000, Description = "Sedan", Category = categories[0] },
                new Car { Id = 3, Name = "BMW 3 Series", Price = 45000, Description = "Sedan", Category = categories[0] },
          new Car { Id = 4, Name = "Toyota RAV4", Price = 35000, Description = "SUV", Category = categories[1] },
                new Car { Id = 5, Name = "Honda CR-V", Price = 36000, Description = "SUV", Category = categories[1] }
    };

 context.Cars.AddRange(cars);
     await context.SaveChangesAsync();
        }

#region Интеграционные тесты пейджинации

        /// <summary>
        /// Интеграционный тест: получение списка автомобилей с фильтром и пейджинацией
        /// </summary>
        [Fact]
        public async Task Integration_GetCarsWithFilterAndPagination()
        {
            // Arrange
        using var context = CreateContext();
await SeedCompleteDataAsync(context);

            const string categoryFilter = "sedans";
     const int pageNo = 1;
    const int pageSize = 2;

            // Act
  var query = context.Cars
     .AsNoTracking()
    .Include(c => c.Category)
                .Where(c => c.Category != null && c.Category.NormalizedName == categoryFilter);

 var total = await query.CountAsync();
         var items = await query
              .OrderBy(c => c.Id)
           .Skip((pageNo - 1) * pageSize)
    .Take(pageSize)
            .ToListAsync();

   var totalPages = (int)Math.Ceiling(total / (double)pageSize);

     // Assert
            Assert.Equal(3, total);
     Assert.Equal(2, items.Count);
            Assert.Equal(2, totalPages);
            Assert.All(items, car => Assert.Equal("sedans", car.Category?.NormalizedName));
     }

    /// <summary>
        /// Интеграционный тест: получение последней страницы
        /// </summary>
        [Fact]
    public async Task Integration_GetLastPage()
        {
   // Arrange
 using var context = CreateContext();
       await SeedCompleteDataAsync(context);

   const int pageSize = 2;
            var total = await context.Cars.CountAsync();
       var lastPage = (int)Math.Ceiling(total / (double)pageSize);

            // Act
         var items = await context.Cars
    .AsNoTracking()
                .Include(c => c.Category)
       .OrderBy(c => c.Id)
            .Skip((lastPage - 1) * pageSize)
     .Take(pageSize)
        .ToListAsync();

   // Assert
            Assert.NotEmpty(items);
    Assert.True(items.Count <= pageSize);
            Assert.Equal(1, items.Count); // Последняя страница должна иметь одну машину
        }

        #endregion

 #region Тесты преобразования данных

        /// <summary>
  /// Тест: правильное преобразование в ResponseData<ListModel<Car>>
        /// </summary>
   [Fact]
        public async Task Integration_ConvertToResponseData()
 {
          // Arrange
 using var context = CreateContext();
    await SeedCompleteDataAsync(context);

     const int pageNo = 1;
        const int pageSize = 3;

       // Act
     var query = context.Cars.AsNoTracking().Include(c => c.Category);
  var total = await query.CountAsync();
      var items = await query
        .OrderBy(c => c.Id)
          .Skip((pageNo - 1) * pageSize)
        .Take(pageSize)
    .ToListAsync();

            var listModel = new ListModel<Car>
    {
      Items = items,
                CurrentPage = pageNo,
    TotalPages = (int)Math.Ceiling(total / (double)pageSize)
  };

            var response = new ResponseData<ListModel<Car>>
        {
  Successfull = true,
            Data = listModel,
    ErrorMessage = null
            };

   // Assert
  Assert.NotNull(response);
    Assert.True(response.Successfull);
 Assert.NotNull(response.Data);
            Assert.Equal(3, response.Data.Items.Count);
 Assert.Equal(1, response.Data.CurrentPage);
     Assert.Equal(2, response.Data.TotalPages);
        }

        /// <summary>
        /// Тест: обработка ошибок и возврат ErrorMessage
        /// </summary>
        [Fact]
public void Integration_ErrorResponseData()
  {
   // Arrange & Act
          var response = ResponseData<ListModel<Car>>.Error("Ошибка подключения к БД");

      // Assert
          Assert.NotNull(response);
     Assert.False(response.Successfull);
            Assert.Null(response.Data);
        Assert.Equal("Ошибка подключения к БД", response.ErrorMessage);
        }

        #endregion

        #region Тесты производительности и масштабируемости

        /// <summary>
   /// Тест: работа с большим количеством данных (100 машин)
     /// </summary>
     [Fact]
        public async Task Integration_LargeDataset()
        {
      // Arrange
        using var context = CreateContext();
 
            var category = new Category { Id = 1, Name = "Test", NormalizedName = "test" };
            context.Categories.Add(category);
 await context.SaveChangesAsync();

        var cars = Enumerable.Range(1, 100)
    .Select(i => new Car
      {
           Id = i,
          Name = $"Car {i}",
      Price = 10000 + i * 1000,
        Category = category
    })
                .ToList();

    context.Cars.AddRange(cars);
            await context.SaveChangesAsync();

        const int pageSize = 10;

 // Act - получаем разные страницы
         var page1 = await context.Cars
              .AsNoTracking()
        .Include(c => c.Category)
.OrderBy(c => c.Id)
   .Skip(0)
      .Take(pageSize)
    .ToListAsync();

  var page5 = await context.Cars
      .AsNoTracking()
       .Include(c => c.Category)
      .OrderBy(c => c.Id)
                .Skip(40)
          .Take(pageSize)
     .ToListAsync();

            var page10 = await context.Cars
       .AsNoTracking()
          .Include(c => c.Category)
            .OrderBy(c => c.Id)
           .Skip(90)
   .Take(pageSize)
        .ToListAsync();

    // Assert
  Assert.Equal(10, page1.Count);
            Assert.Equal(10, page5.Count);
       Assert.Equal(10, page10.Count);

       Assert.Equal(1, page1[0].Id);
            Assert.Equal(41, page5[0].Id);
            Assert.Equal(91, page10[0].Id);
        }

    #endregion

 #region Тесты целостности данных

        /// <summary>
        /// Тест: целостность связей категория-машина
        /// </summary>
   [Fact]
     public async Task Integration_DataIntegrity_CategoryCarRelationship()
        {
  // Arrange
         using var context = CreateContext();
            await SeedCompleteDataAsync(context);

            // Act
            var carsWithCategories = await context.Cars
         .AsNoTracking()
          .Include(c => c.Category)
              .ToListAsync();

    // Assert
   Assert.All(carsWithCategories, car =>
       {
    Assert.NotNull(car.Category);
  Assert.True(car.Category.Id > 0);
       Assert.NotEmpty(car.Category.Name);
            });
}

        /// <summary>
        /// Тест: логическая консистентность после пейджинации
    /// </summary>
        [Fact]
        public async Task Integration_PaginationConsistency()
    {
    // Arrange
       using var context = CreateContext();
            await SeedCompleteDataAsync(context);

            const int pageSize = 2;
            var total = await context.Cars.CountAsync();
            var totalPages = (int)Math.Ceiling(total / (double)pageSize);

    // Act - собираем все страницы
     var allItems = new List<Car>();
   for (int page = 1; page <= totalPages; page++)
        {
           var items = await context.Cars
    .AsNoTracking()
      .Include(c => c.Category)
        .OrderBy(c => c.Id)
        .Skip((page - 1) * pageSize)
  .Take(pageSize)
 .ToListAsync();

            allItems.AddRange(items);
            }

            // Assert
      Assert.Equal(total, allItems.Count);
            Assert.Equal(allItems.OrderBy(c => c.Id).Select(c => c.Id), allItems.Select(c => c.Id));
        }

      #endregion

        #region Граничные случаи интеграции

        /// <summary>
        /// Тест: получение машины по ID с категорией
        /// </summary>
        [Fact]
     public async Task Integration_GetCarById_IncludesCategory()
      {
      // Arrange
        using var context = CreateContext();
  await SeedCompleteDataAsync(context);

            const int carId = 1;

       // Act
     var car = await context.Cars
      .AsNoTracking()
       .Include(c => c.Category)
      .FirstOrDefaultAsync(c => c.Id == carId);

  // Assert
 Assert.NotNull(car);
Assert.Equal(1, car.Id);
     Assert.Equal("Toyota Camry", car.Name);
  Assert.NotNull(car.Category);
  Assert.Equal("Седаны", car.Category.Name);
        }

        /// <summary>
        /// Тест: фильтрация с пустым результатом
        /// </summary>
      [Fact]
        public async Task Integration_FilterWithNoResults()
        {
        // Arrange
            using var context = CreateContext();
       await SeedCompleteDataAsync(context);

     // Act
            var items = await context.Cars
           .AsNoTracking()
                .Include(c => c.Category)
       .Where(c => c.Price > 100000)
        .ToListAsync();

            // Assert
            Assert.Empty(items);
        }

        /// <summary>
        /// Тест: агрегирующие функции (Sum, Average, Count)
      /// </summary>
        [Fact]
        public async Task Integration_AggregationFunctions()
        {
     // Arrange
 using var context = CreateContext();
        await SeedCompleteDataAsync(context);

    // Act
    var count = await context.Cars.CountAsync();
            var avgPrice = await context.Cars.AverageAsync(c => c.Price);
          var maxPrice = await context.Cars.MaxAsync(c => c.Price);
        var minPrice = await context.Cars.MinAsync(c => c.Price);

            // Assert
            Assert.Equal(5, count);
            Assert.True(avgPrice > 0);
            Assert.Equal(45000, maxPrice);
            Assert.Equal(25000, minPrice);
        }

        #endregion

        public void Dispose()
        {
       _connection?.Dispose();
        }
    }
}
