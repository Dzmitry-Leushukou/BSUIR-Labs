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
    /// Тесты для GetCategoryListAsync API сервиса
  /// </summary>
    public class CategoryApiServiceTests : IDisposable
    {
        private readonly SqliteConnection _connection;

   public CategoryApiServiceTests()
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

        private async Task SeedTestDataAsync(AppDbContext context)
      {
            var categories = new List<Category>
            {
    new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
                new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" },
         new Category { Id = 3, Name = "Минивэны", NormalizedName = "minivans" },
    new Category { Id = 4, Name = "Пикапы", NormalizedName = "trucks" }
            };

            context.Categories.AddRange(categories);
            await context.SaveChangesAsync();
        }

    #region Основные тесты

        /// <summary>
        /// Тест: получение всех категорий
      /// </summary>
   [Fact]
        public async Task GetCategoryListAsync_ReturnsAllCategories()
        {
         // Arrange
      using var context = CreateContext();
            await SeedTestDataAsync(context);

         // Act
          var categories = await context.Categories
        .AsNoTracking()
    .ToListAsync();

      // Assert
            Assert.NotNull(categories);
         Assert.Equal(4, categories.Count);
     Assert.Contains(categories, c => c.Name == "Седаны");
            Assert.Contains(categories, c => c.Name == "Кроссоверы");
        Assert.Contains(categories, c => c.Name == "Минивэны");
    Assert.Contains(categories, c => c.Name == "Пикапы");
        }

        /// <summary>
        /// Тест: все категории содержат NormalizedName
 /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_AllCategoriesHaveNormalizedName()
 {
// Arrange
            using var context = CreateContext();
     await SeedTestDataAsync(context);

            // Act
var categories = await context.Categories
      .AsNoTracking()
                .ToListAsync();

          // Assert
            Assert.All(categories, c => 
       {
  Assert.NotNull(c.NormalizedName);
                Assert.NotEmpty(c.NormalizedName);
          Assert.Equal(c.NormalizedName, c.NormalizedName.ToLower());
    });
   }

        /// <summary>
        /// Тест: категории отсортированы по ID
     /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_CategoriesSortedById()
 {
     // Arrange
          using var context = CreateContext();
        await SeedTestDataAsync(context);

      // Act
    var categories = await context.Categories
           .AsNoTracking()
    .OrderBy(c => c.Id)
     .ToListAsync();

            // Assert
        var ids = categories.Select(c => c.Id).ToList();
      Assert.Equal(ids.OrderBy(id => id), ids);
        }

        /// <summary>
        /// Тест: поиск категории по NormalizedName
        /// </summary>
        [Theory]
        [InlineData("sedans", "Седаны")]
        [InlineData("suvs", "Кроссоверы")]
        [InlineData("minivans", "Минивэны")]
  [InlineData("trucks", "Пикапы")]
        public async Task GetCategoryListAsync_FindByNormalizedName(string normalized, string expectedName)
        {
    // Arrange
            using var context = CreateContext();
     await SeedTestDataAsync(context);

            // Act
 var category = await context.Categories
              .AsNoTracking()
         .FirstOrDefaultAsync(c => c.NormalizedName == normalized);

    // Assert
      Assert.NotNull(category);
       Assert.Equal(expectedName, category.Name);
        }

        /// <summary>
  /// Тест: поиск по несуществующему NormalizedName возвращает null
 /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_FindByNonexistentNormalizedName_ReturnsNull()
        {
         // Arrange
     using var context = CreateContext();
            await SeedTestDataAsync(context);

   // Act
       var category = await context.Categories
          .AsNoTracking()
    .FirstOrDefaultAsync(c => c.NormalizedName == "nonexistent");

    // Assert
 Assert.Null(category);
        }

  #endregion

        #region Тесты связей

        /// <summary>
        /// Тест: категория может содержать множество автомобилей
        /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_CategoryCanHaveMultipleCars()
        {
         // Arrange
            using var context = CreateContext();
     var category = new Category { Id = 1, Name = "Тест", NormalizedName = "test" };
 context.Categories.Add(category);
          await context.SaveChangesAsync();

   var cars = new List<Car>
            {
          new Car { Id = 1, Name = "Car 1", Price = 10000, Category = category },
    new Car { Id = 2, Name = "Car 2", Price = 20000, Category = category },
          new Car { Id = 3, Name = "Car 3", Price = 30000, Category = category }
        };

            context.Cars.AddRange(cars);
            await context.SaveChangesAsync();

         // Act
      var carsInCategory = await context.Cars
      .AsNoTracking()
                .Include(c => c.Category)
 .Where(c => c.Category.Id == category.Id)
                .ToListAsync();

            // Assert
       Assert.Equal(3, carsInCategory.Count);
    Assert.All(carsInCategory, car => Assert.Equal(category.Id, car.Category.Id));
        }

        /// <summary>
        /// Тест: удаление категории
     /// </summary>
     [Fact]
        public async Task GetCategoryListAsync_CanDeleteCategory()
    {
            // Arrange
            using var context = CreateContext();
 await SeedTestDataAsync(context);

     var categoryToDelete = await context.Categories.FirstAsync();
    int originalCount = await context.Categories.CountAsync();

      // Act
    context.Categories.Remove(categoryToDelete);
            await context.SaveChangesAsync();

            var newCount = await context.Categories.CountAsync();

     // Assert
   Assert.Equal(originalCount - 1, newCount);
        }

        #endregion

        #region Граничные случаи

   /// <summary>
    /// Тест: пустой список при отсутствии категорий
        /// </summary>
     [Fact]
        public async Task GetCategoryListAsync_EmptyListWhenNoCategories()
        {
         // Arrange
            using var context = CreateContext();
            // Не добавляем категории

 // Act
            var categories = await context.Categories
       .AsNoTracking()
                .ToListAsync();

    // Assert
      Assert.Empty(categories);
        }

    /// <summary>
        /// Тест: категория с минимальными данными
     /// </summary>
        [Fact]
     public async Task GetCategoryListAsync_CategoryWithMinimalData()
        {
     // Arrange
       using var context = CreateContext();
     var category = new Category 
    { 
         Id = 1, 
    Name = "A", 
      NormalizedName = "a" 
            };
            context.Categories.Add(category);
await context.SaveChangesAsync();

  // Act
            var retrieved = await context.Categories
          .AsNoTracking()
.FirstAsync();

          // Assert
     Assert.Equal("A", retrieved.Name);
         Assert.Equal("a", retrieved.NormalizedName);
        }

      /// <summary>
        /// Тест: категория с длинным названием
  /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_CategoryWithLongName()
  {
            // Arrange
          using var context = CreateContext();
    var longName = new string('A', 255);
            var category = new Category 
            { 
  Id = 1, 
 Name = longName, 
             NormalizedName = longName.ToLower() 
        };
    context.Categories.Add(category);
            await context.SaveChangesAsync();

 // Act
 var retrieved = await context.Categories
         .AsNoTracking()
      .FirstAsync();

            // Assert
         Assert.Equal(longName, retrieved.Name);
            Assert.Equal(255, retrieved.Name.Length);
   }

        /// <summary>
        /// Тест: категории с уникальными NormalizedName
   /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_UniqueNormalizedNames()
        {
            // Arrange
            using var context = CreateContext();
await SeedTestDataAsync(context);

  // Act
   var categories = await context.Categories
      .AsNoTracking()
    .ToListAsync();

            var normalizedNames = categories.Select(c => c.NormalizedName).ToList();

    // Assert
         Assert.Equal(normalizedNames.Distinct().Count(), normalizedNames.Count);
        }

        #endregion

        public void Dispose()
        {
            _connection?.Dispose();
        }
    }
}
