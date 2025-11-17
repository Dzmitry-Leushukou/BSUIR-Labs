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

namespace Lab1.Tests.API.Examples
{
    /// <summary>
  /// Примеры использования SQLite in-memory для тестирования
    /// Задание №2 - Рекомендации по тестированию
    /// </summary>
    public class SqliteInMemoryExamples
    {
        // ============ ПРИМЕР 1: Базовая настройка ============

        /// <summary>
     /// Пример создания контекста с SQLite in-memory
        /// </summary>
 [Fact]
        public void Example1_CreateSqliteInMemoryContext()
        {
    // Шаг 1: Создаём соединение SQLite in-memory
            var connection = new SqliteConnection("Filename=:memory:");
            connection.Open();

            try
 {
           // Шаг 2: Создаём DbContext options
  var options = new DbContextOptionsBuilder<AppDbContext>()
     .UseSqlite(connection)
   .Options;

         // Шаг 3: Создаём контекст
   using var context = new AppDbContext(options);

            // Шаг 4: Инициализируем БД (создаём таблицы)
        context.Database.EnsureCreated();

         // Проверяем что БД создана
         Assert.NotNull(context);
          }
  finally
            {
           connection.Close();
      connection.Dispose();
 }
        }

        // ============ ПРИМЕР 2: С использованием IDisposable ============

        /// <summary>
        /// Пример класса тестов с правильным управлением ресурсами
      /// </summary>
        public class TestClassExample : IDisposable
        {
    private readonly SqliteConnection _connection;

public TestClassExample()
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

   [Fact]
            public async Task TestMethod()
            {
              using var context = CreateContext();

        // Добавляем тестовые данные
var category = new Category { Id = 1, Name = "Test", NormalizedName = "test" };
        context.Categories.Add(category);
await context.SaveChangesAsync();

      // Проверяем
    var retrieved = await context.Categories.FirstAsync();
          Assert.Equal("Test", retrieved.Name);
            }

         public void Dispose()
            {
       _connection?.Dispose();
      }
        }

        // ============ ПРИМЕР 3: Добавление и получение данных ============

      [Fact]
        public async Task Example3_AddAndRetrieveData()
        {
    var connection = new SqliteConnection("Filename=:memory:");
          connection.Open();

            try
       {
           var options = new DbContextOptionsBuilder<AppDbContext>()
  .UseSqlite(connection)
  .Options;

          using (var context = new AppDbContext(options))
         {
         context.Database.EnsureCreated();

     // Добавляем категорию
 var category = new Category
            {
     Id = 1,
         Name = "Седаны",
          NormalizedName = "sedans"
   };

          context.Categories.Add(category);
         await context.SaveChangesAsync();
   }

         // В новом контексте получаем данные
             using (var context = new AppDbContext(options))
        {
        var category = await context.Categories.FirstAsync();
        Assert.Equal("Седаны", category.Name);
 Assert.Equal("sedans", category.NormalizedName);
  }
   }
            finally
        {
      connection.Close();
          connection.Dispose();
            }
        }

   // ============ ПРИМЕР 4: Пейджинация ============

        [Fact]
public async Task Example4_Pagination()
    {
     var connection = new SqliteConnection("Filename=:memory:");
 connection.Open();

     try
         {
        var options = new DbContextOptionsBuilder<AppDbContext>()
     .UseSqlite(connection)
          .Options;

using (var context = new AppDbContext(options))
     {
context.Database.EnsureCreated();

         // Добавляем 10 машин
                  var cars = Enumerable.Range(1, 10)
              .Select(i => new Car
         {
         Id = i,
  Name = $"Car {i}",
            Price = 10000 + i * 1000,
        ImagePath = "noimage.jpg"
   })
           .ToList();

           context.Cars.AddRange(cars);
         await context.SaveChangesAsync();
          }

      using (var context = new AppDbContext(options))
 {
       // Получаем страницу 1: 3 машины
   const int pageSize = 3;
        const int pageNo = 1;

         var items = await context.Cars
              .AsNoTracking()
       .OrderBy(c => c.Id)
      .Skip((pageNo - 1) * pageSize)
          .Take(pageSize)
             .ToListAsync();

   Assert.Equal(3, items.Count);
          Assert.Equal(1, items[0].Id);
           Assert.Equal(2, items[1].Id);
              Assert.Equal(3, items[2].Id);

         // Получаем страницу 2: 3 машины
         var page2Items = await context.Cars
  .AsNoTracking()
              .OrderBy(c => c.Id)
            .Skip((2 - 1) * pageSize)
     .Take(pageSize)
    .ToListAsync();

   Assert.Equal(3, page2Items.Count);
              Assert.Equal(4, page2Items[0].Id);
  }
        }
      finally
          {
         connection.Close();
       connection.Dispose();
    }
        }

   // ============ ПРИМЕР 5: Фильтрация ============

        [Fact]
        public async Task Example5_Filtering()
        {
            var connection = new SqliteConnection("Filename=:memory:");
    connection.Open();

      try
    {
            var options = new DbContextOptionsBuilder<AppDbContext>()
         .UseSqlite(connection)
   .Options;

                using (var context = new AppDbContext(options))
       {
             context.Database.EnsureCreated();

          // Добавляем категории
      var sedans = new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" };
     var suvs = new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" };

      context.Categories.AddRange(sedans, suvs);
await context.SaveChangesAsync();

          // Добавляем машины разных категорий
           var cars = new List<Car>
       {
         new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = sedans },
      new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = sedans },
              new Car { Id = 3, Name = "Toyota RAV4", Price = 35000, Category = suvs },
   new Car { Id = 4, Name = "Honda CR-V", Price = 36000, Category = suvs }
        };

 context.Cars.AddRange(cars);
  await context.SaveChangesAsync();
    }

     using (var context = new AppDbContext(options))
                {
            // Фильтруем по категории
        var sedanCars = await context.Cars
   .AsNoTracking()
               .Include(c => c.Category)
   .Where(c => c.Category.NormalizedName == "sedans")
 .ToListAsync();

           Assert.Equal(2, sedanCars.Count);
             Assert.All(sedanCars, car => Assert.Equal("sedans", car.Category.NormalizedName));
       }
   }
    finally
            {
       connection.Close();
        connection.Dispose();
      }
        }

        // ============ ПРИМЕР 6: Вычисления ============

        [Fact]
        public async Task Example6_Calculations()
     {
    var connection = new SqliteConnection("Filename=:memory:");
      connection.Open();

            try
  {
 var options = new DbContextOptionsBuilder<AppDbContext>()
        .UseSqlite(connection)
    .Options;

 using (var context = new AppDbContext(options))
      {
        context.Database.EnsureCreated();

     // Добавляем машины с разными ценами
              var cars = new List<Car>
               {
  new Car { Id = 1, Name = "Cheap", Price = 10000 },
      new Car { Id = 2, Name = "Mid", Price = 20000 },
 new Car { Id = 3, Name = "Expensive", Price = 30000 }
        };

           context.Cars.AddRange(cars);
   await context.SaveChangesAsync();
         }

 using (var context = new AppDbContext(options))
     {
        // Вычисляем
    var count = await context.Cars.CountAsync();
       var avgPrice = await context.Cars.AverageAsync(c => c.Price);
        var maxPrice = await context.Cars.MaxAsync(c => c.Price);
           var minPrice = await context.Cars.MinAsync(c => c.Price);
            var totalPrice = await context.Cars.SumAsync(c => c.Price);

              Assert.Equal(3, count);
       Assert.Equal(20000, avgPrice);
       Assert.Equal(30000, maxPrice);
           Assert.Equal(10000, minPrice);
       Assert.Equal(60000, totalPrice);
            }
   }
         finally
    {
     connection.Close();
         connection.Dispose();
            }
        }

        // ============ ПРИМЕР 7: Обновление данных ============

        [Fact]
        public async Task Example7_UpdateData()
        {
            var connection = new SqliteConnection("Filename=:memory:");
 connection.Open();

     try
    {
           var options = new DbContextOptionsBuilder<AppDbContext>()
        .UseSqlite(connection)
               .Options;

      using (var context = new AppDbContext(options))
   {
      context.Database.EnsureCreated();

var car = new Car { Id = 1, Name = "Old Name", Price = 10000 };
            context.Cars.Add(car);
 await context.SaveChangesAsync();
       }

                using (var context = new AppDbContext(options))
  {
    var car = await context.Cars.FirstAsync();
           
              // Обновляем
   car.Name = "New Name";
     car.Price = 20000;
           
           context.Cars.Update(car);
      await context.SaveChangesAsync();
        }

              using (var context = new AppDbContext(options))
     {
   var car = await context.Cars.FirstAsync();
        Assert.Equal("New Name", car.Name);
    Assert.Equal(20000, car.Price);
                }
            }
      finally
      {
        connection.Close();
   connection.Dispose();
          }
        }

        // ============ ПРИМЕР 8: Удаление данных ============

        [Fact]
     public async Task Example8_DeleteData()
        {
            var connection = new SqliteConnection("Filename=:memory:");
     connection.Open();

            try
  {
        var options = new DbContextOptionsBuilder<AppDbContext>()
        .UseSqlite(connection)
         .Options;

       using (var context = new AppDbContext(options))
                {
     context.Database.EnsureCreated();

        var cars = new List<Car>
         {
       new Car { Id = 1, Name = "Car 1", Price = 10000 },
       new Car { Id = 2, Name = "Car 2", Price = 20000 }
      };

                  context.Cars.AddRange(cars);
                 await context.SaveChangesAsync();
     }

     using (var context = new AppDbContext(options))
        {
      var count1 = await context.Cars.CountAsync();
         Assert.Equal(2, count1);

  var carToDelete = await context.Cars.FirstAsync();
       context.Cars.Remove(carToDelete);
     await context.SaveChangesAsync();
   }

                using (var context = new AppDbContext(options))
      {
         var count2 = await context.Cars.CountAsync();
          Assert.Equal(1, count2);
       }
    }
            finally
   {
      connection.Close();
   connection.Dispose();
            }
        }

      // ============ ПРИМЕР 9: Отладка SQL запросов ============

        [Fact]
        public async Task Example9_DebugSqlQueries()
        {
            var connection = new SqliteConnection("Filename=:memory:");
     connection.Open();

         try
  {
           var options = new DbContextOptionsBuilder<AppDbContext>()
       .UseSqlite(connection)
             .LogTo(Console.WriteLine) // Логируем SQL запросы
             .Options;

        using (var context = new AppDbContext(options))
     {
     context.Database.EnsureCreated();

        var cars = new List<Car>
          {
          new Car { Id = 1, Name = "Car 1", Price = 10000 },
             new Car { Id = 2, Name = "Car 2", Price = 20000 }
   };

         context.Cars.AddRange(cars);
          await context.SaveChangesAsync();

  // Этот запрос будет залогирован
         var result = await context.Cars
          .Where(c => c.Price > 15000)
    .ToListAsync();

   Assert.Single(result);
     }
            }
       finally
 {
     connection.Close();
        connection.Dispose();
            }
   }

        // ============ ПРИМЕР 10: Сложный интеграционный тест ============

        [Fact]
        public async Task Example10_ComplexIntegrationTest()
        {
            var connection = new SqliteConnection("Filename=:memory:");
            connection.Open();

         try
    {
      var options = new DbContextOptionsBuilder<AppDbContext>()
            .UseSqlite(connection)
        .Options;

 // Шаг 1: Создаём данные
       using (var context = new AppDbContext(options))
        {
             context.Database.EnsureCreated();

     var sedans = new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" };
               context.Categories.Add(sedans);
               await context.SaveChangesAsync();

            var cars = Enumerable.Range(1, 6)
        .Select(i => new Car
  {
          Id = i,
    Name = $"Sedan {i}",
    Price = 25000 + i * 1000,
        Category = sedans
           })
            .ToList();

        context.Cars.AddRange(cars);
       await context.SaveChangesAsync();
}

          // Шаг 2: Получаем пейджированные данные
    using (var context = new AppDbContext(options))
                {
             const int pageSize = 2;
     var totalItems = await context.Cars.CountAsync();
  var totalPages = (int)Math.Ceiling(totalItems / (double)pageSize);

          for (int page = 1; page <= totalPages; page++)
          {
 var items = await context.Cars
           .AsNoTracking()
        .Include(c => c.Category)
 .Where(c => c.Category.NormalizedName == "sedans")
           .OrderBy(c => c.Id)
         .Skip((page - 1) * pageSize)
    .Take(pageSize)
       .ToListAsync();

 // Проверяем каждую страницу
             if (page < totalPages)
        {
         Assert.Equal(2, items.Count);
           }
        }
     }

           // Шаг 3: Проверяем финальное состояние
                using (var context = new AppDbContext(options))
     {
   var carCount = await context.Cars.CountAsync();
         var categoryCount = await context.Categories.CountAsync();

          Assert.Equal(6, carCount);
           Assert.Equal(1, categoryCount);
    }
  }
      finally
   {
           connection.Close();
                connection.Dispose();
            }
        }
    }
}
