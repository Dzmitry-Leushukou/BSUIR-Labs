using Lab1.API.Data;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace Lab1.Tests.API
{
    /// <summary>
  /// Модульные тесты для API endpoints
    /// </summary>
    public class CarEndpointsTests
    {
        private AppDbContext GetInMemoryDbContext()
{
        var options = new DbContextOptionsBuilder<AppDbContext>()
   .UseInMemoryDatabase(databaseName: Guid.NewGuid().ToString())
         .Options;

          return new AppDbContext(options);
        }

        /// <summary>
        /// Тест: проверка структуры Car entity
        /// </summary>
        [Fact]
        public void Car_HasAllRequiredProperties()
        {
         // Arrange & Act
          var car = new Car
     {
          Id = 1,
                Name = "Test Car",
    Description = "Test Description",
       Price = 25000.99,
      ImagePath = "/images/test.jpg",
 Mime = "image/jpeg",
     Category = new Category { Id = 1, Name = "Sedans" }
    };

// Assert
            Assert.Equal(1, car.Id);
          Assert.Equal("Test Car", car.Name);
    Assert.Equal("Test Description", car.Description);
            Assert.Equal(25000.99, car.Price);
      Assert.Equal("/images/test.jpg", car.ImagePath);
            Assert.Equal("image/jpeg", car.Mime);
  Assert.NotNull(car.Category);
 Assert.Equal("Sedans", car.Category.Name);
        }

        /// <summary>
        /// Тест: проверка структуры Category entity
    /// </summary>
      [Fact]
        public void Category_HasAllRequiredProperties()
        {
            // Arrange & Act
          var category = new Category
            {
   Id = 1,
          Name = "Sedans",
      NormalizedName = "sedans"
         };

      // Assert
        Assert.Equal(1, category.Id);
        Assert.Equal("Sedans", category.Name);
            Assert.Equal("sedans", category.NormalizedName);
        }

        /// <summary>
        /// Тест: проверка работы DbContext с InMemory базой
        /// </summary>
        [Fact]
public async Task DbContext_CanAddAndRetrieveCar()
    {
            // Arrange
         using (var context = GetInMemoryDbContext())
    {
       var category = new Category { Id = 1, Name = "Sedans", NormalizedName = "sedans" };
 var car = new Car
     {
Id = 1,
    Name = "Toyota Camry",
       Price = 25000,
  Category = category
  };

     // Act
     context.Categories.Add(category);
         context.Cars.Add(car);
          await context.SaveChangesAsync();

  var retrieved = await context.Cars.Include(c => c.Category).FirstOrDefaultAsync(c => c.Id == 1);

     // Assert
       Assert.NotNull(retrieved);
                Assert.Equal("Toyota Camry", retrieved.Name);
         Assert.Equal(25000, retrieved.Price);
        Assert.NotNull(retrieved.Category);
     Assert.Equal("Sedans", retrieved.Category.Name);
       }
}

        /// <summary>
    /// Тест: проверка фильтрации по категориям
   /// </summary>
 [Fact]
        public async Task DbContext_CanFilterCarsByCategory()
{
            // Arrange
   using (var context = GetInMemoryDbContext())
     {
    var sedans = new Category { Id = 1, Name = "Sedans", NormalizedName = "sedans" };
          var suvs = new Category { Id = 2, Name = "SUVs", NormalizedName = "suvs" };

                context.Categories.AddRange(sedans, suvs);

        var cars = new List<Car>
        {
        new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = sedans },
           new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = sedans },
        new Car { Id = 3, Name = "Toyota RAV4", Price = 35000, Category = suvs }
       };

        context.Cars.AddRange(cars);
  await context.SaveChangesAsync();

     // Act
             var sedanCars = await context.Cars
       .AsNoTracking()
               .Include(c => c.Category)
      .Where(c => c.Category.NormalizedName == "sedans")
.ToListAsync();

          // Assert
   Assert.Equal(2, sedanCars.Count);
         Assert.All(sedanCars, c => Assert.Equal("sedans", c.Category.NormalizedName));
  }
        }

        /// <summary>
        /// Тест: проверка пейджинации в DbContext
    /// </summary>
        [Fact]
        public async Task DbContext_CanPageCars()
     {
        // Arrange
     using (var context = GetInMemoryDbContext())
     {
   var category = new Category { Id = 1, Name = "Test", NormalizedName = "test" };
  context.Categories.Add(category);

       var cars = Enumerable.Range(1, 10)
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

      // Act - страница 1, по 3 авто
           var page1 = await context.Cars
   .AsNoTracking()
      .Include(c => c.Category)
          .OrderBy(c => c.Id)
       .Skip(0)
       .Take(3)
          .ToListAsync();

    // Act - страница 2, по 3 авто
                var page2 = await context.Cars
              .AsNoTracking()
       .Include(c => c.Category)
 .OrderBy(c => c.Id)
      .Skip(3)
     .Take(3)
           .ToListAsync();

     // Assert
     Assert.Equal(3, page1.Count);
         Assert.Equal(3, page2.Count);
           Assert.Equal("Car 1", page1[0].Name);
   Assert.Equal("Car 4", page2[0].Name);
 }
     }

        /// <summary>
        /// Тест: проверка ListModel структуры
        /// </summary>
        [Fact]
        public void ListModel_HasCorrectStructure()
        {
    // Arrange
  var cars = new List<Car>
            {
            new Car { Id = 1, Name = "Car 1", Price = 10000 },
        new Car { Id = 2, Name = "Car 2", Price = 20000 }
    };

         // Act
    var listModel = new ListModel<Car>
          {
              Items = cars,
         CurrentPage = 1,
           TotalPages = 5
   };

            // Assert
Assert.NotNull(listModel.Items);
            Assert.Equal(2, listModel.Items.Count);
    Assert.Equal(1, listModel.CurrentPage);
     Assert.Equal(5, listModel.TotalPages);
        }

        /// <summary>
     /// Тест: проверка ResponseData структуры
        /// </summary>
        [Fact]
        public void ResponseData_CanBothSuccessAndError()
 {
            // Arrange & Act - успех
            var successResponse = new ResponseData<List<Car>>
   {
            Successfull = true,
      Data = new List<Car> { new Car { Id = 1, Name = "Test" } },
                ErrorMessage = null
            };

   // Assert - успех
        Assert.True(successResponse.Successfull);
    Assert.NotNull(successResponse.Data);
 Assert.Null(successResponse.ErrorMessage);

   // Arrange & Act - ошибка
            var errorResponse = new ResponseData<List<Car>>
        {
         Successfull = false,
    Data = null,
   ErrorMessage = "Error occurred"
     };

  // Assert - ошибка
  Assert.False(errorResponse.Successfull);
      Assert.Null(errorResponse.Data);
            Assert.NotNull(errorResponse.ErrorMessage);
 }

        /// <summary>
     /// Тест: проверка null-безопасности при работе с категориями
     /// </summary>
        [Fact]
   public async Task DbContext_HandlesNullCategoryGracefully()
        {
   // Arrange
   using (var context = GetInMemoryDbContext())
  {
                var car = new Car { Id = 1, Name = "Car without category", Price = 15000, Category = null };
     context.Cars.Add(car);
    await context.SaveChangesAsync();

      // Act
      var retrieved = await context.Cars
   .AsNoTracking()
   .Include(c => c.Category)
     .FirstOrDefaultAsync(c => c.Id == 1);

   // Assert
      Assert.NotNull(retrieved);
 Assert.Equal("Car without category", retrieved.Name);
            Assert.Null(retrieved.Category);
  }
        }
    }
}
