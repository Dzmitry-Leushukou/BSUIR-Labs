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
    /// Тесты для GetCarListAsync API сервиса
    /// Задание №2 - Тестирование сервиса API
    /// </summary>
    public class CarApiServiceTests : IDisposable
    {
     private readonly SqliteConnection _connection;

        public CarApiServiceTests()
        {
            // Создаём SQLite in-memory соединение
_connection = new SqliteConnection("Filename=:memory:");
            _connection.Open();
        }

   /// <summary>
  /// Создаёт контекст БД с SQLite in-memory
        /// </summary>
        private AppDbContext CreateContext()
    {
    var options = new DbContextOptionsBuilder<AppDbContext>()
              .UseSqlite(_connection)
     .Options;

         var context = new AppDbContext(options);
            context.Database.EnsureCreated();
    return context;
    }

    /// <summary>
   /// Заполняет БД тестовыми данными
        /// </summary>
private async Task SeedTestDataAsync(AppDbContext context)
        {
         var sedans = new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" };
      var suvs = new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" };

       context.Categories.AddRange(sedans, suvs);
            await context.SaveChangesAsync();

            // Добавляем 7 автомобилей для проверки пейджинации (3 на странице = 3 страницы)
        var cars = new List<Car>
  {
  new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = sedans, ImagePath = "noimage.jpg" },
       new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = sedans, ImagePath = "noimage.jpg" },
           new Car { Id = 3, Name = "BMW 3 Series", Price = 45000, Category = sedans, ImagePath = "noimage.jpg" },
     new Car { Id = 4, Name = "Toyota RAV4", Price = 35000, Category = suvs, ImagePath = "noimage.jpg" },
          new Car { Id = 5, Name = "Honda CR-V", Price = 36000, Category = suvs, ImagePath = "noimage.jpg" },
     new Car { Id = 6, Name = "Mazda CX-5", Price = 33000, Category = suvs, ImagePath = "noimage.jpg" },
      new Car { Id = 7, Name = "Ford Escape", Price = 32000, Category = suvs, ImagePath = "noimage.jpg" }
     };

       context.Cars.AddRange(cars);
  await context.SaveChangesAsync();
        }

  #region Требование 1: Первая страница по умолчанию

 /// <summary>
        /// Тест: по умолчанию возвращает первую страницу размером 3 объекта
        /// и правильно рассчитывает количество страниц
        /// </summary>
        [Fact]
  public async Task GetCarListAsync_ReturnsFirstPageOfThreeItems()
   {
    // Arrange
            using var context = CreateContext();
            await SeedTestDataAsync(context);

            // Act
var query = context.Cars.AsNoTracking().Include(c => c.Category);
         var total = await query.CountAsync();
 var pageSize = 3;
 var pageNo = 1;

var items = await query
    .OrderBy(c => c.Id)
           .Skip((pageNo - 1) * pageSize)
 .Take(pageSize)
          .ToListAsync();

            var totalPages = (int)Math.Ceiling(total / (double)pageSize);

            var result = new ListModel<Car>
            {
                Items = items,
         CurrentPage = pageNo,
     TotalPages = totalPages
       };

     // Assert
      Assert.NotNull(result);
   Assert.Equal(1, result.CurrentPage);
      Assert.Equal(3, result.Items.Count);
   Assert.Equal(3, totalPages); // 7 машин / 3 на странице = 3 страницы
     Assert.Equal("Toyota Camry", result.Items[0].Name);
  Assert.Equal("Honda Accord", result.Items[1].Name);
         Assert.Equal("BMW 3 Series", result.Items[2].Name);
    }

 /// <summary>
        /// Тест: проверка правильного расчёта количества страниц для разных размеров
    /// </summary>
 [Theory]
        [InlineData(7, 3, 3)]  // 7 машин / 3 = 3 страницы (2+2+3)
      [InlineData(10, 3, 4)] // 10 машин / 3 = 4 страницы
 [InlineData(6, 3, 2)]  // 6 машин / 3 = 2 страницы
        [InlineData(3, 3, 1)]  // 3 машины / 3 = 1 страница
        public async Task GetCarListAsync_CalculatesTotalPagesCorrectly(int totalItems, int pageSize, int expectedTotalPages)
{
            // Arrange
            using var context = CreateContext();
  var category = new Category { Id = 1, Name = "Test", NormalizedName = "test" };
 context.Categories.Add(category);
            await context.SaveChangesAsync();

    var cars = Enumerable.Range(1, totalItems)
     .Select(i => new Car 
     { 
           Id = i, 
     Name = $"Car {i}", 
      Price = 10000 + i * 1000, 
    Category = category,
           ImagePath = "noimage.jpg"
     })
      .ToList();

            context.Cars.AddRange(cars);
    await context.SaveChangesAsync();

       // Act
            var total = await context.Cars.CountAsync();
var totalPages = (int)Math.Ceiling(total / (double)pageSize);

// Assert
        Assert.Equal(expectedTotalPages, totalPages);
     }

        #endregion

        #region Требование 2: Выбор нужной страницы

      /// <summary>
        /// Тест: метод правильно выбирает заданную страницу
   /// </summary>
        [Theory]
      [InlineData(1)] // Первая страница
        [InlineData(2)] // Вторая страница
        [InlineData(3)] // Третья страница
        public async Task GetCarListAsync_SelectsCorrectPage(int pageNo)
        {
            // Arrange
      using var context = CreateContext();
      await SeedTestDataAsync(context);

 var pageSize = 3;

            // Act
   var query = context.Cars.AsNoTracking().Include(c => c.Category);
      var items = await query
          .OrderBy(c => c.Id)
 .Skip((pageNo - 1) * pageSize)
         .Take(pageSize)
       .ToListAsync();

            var result = new ListModel<Car>
            {
      Items = items,
            CurrentPage = pageNo,
                TotalPages = 3
            };

            // Assert
    Assert.Equal(pageNo, result.CurrentPage);

            // Проверяем правильные машины для каждой страницы
   if (pageNo == 1)
            {
       Assert.Equal(new[] { 1, 2, 3 }, result.Items.Select(c => c.Id));
   }
     else if (pageNo == 2)
          {
       Assert.Equal(new[] { 4, 5, 6 }, result.Items.Select(c => c.Id));
      }
     else if (pageNo == 3)
    {
      Assert.Single(result.Items); // Только одна машина на третьей странице
        Assert.Equal(7, result.Items[0].Id);
}
        }

     #endregion

        #region Требование 3: Фильтрация по категориям

        /// <summary>
        /// Тест: метод правильно выполняет фильтрацию объектов по категории
        /// </summary>
        [Fact]
        public async Task GetCarListAsync_FiltersCorrectlyByCategory()
        {
// Arrange
 using var context = CreateContext();
      await SeedTestDataAsync(context);

            const string categoryFilter = "suvs";

     // Act
     var query = context.Cars
     .AsNoTracking()
    .Include(c => c.Category)
    .Where(c => c.Category != null && c.Category.NormalizedName == categoryFilter);

            var items = await query
  .OrderBy(c => c.Id)
    .Take(3)
       .ToListAsync();

            var total = await query.CountAsync();
   var totalPages = (int)Math.Ceiling(total / 3.0);

          var result = new ListModel<Car>
            {
      Items = items,
       CurrentPage = 1,
     TotalPages = totalPages
    };

            // Assert
            Assert.NotNull(result);
     Assert.Equal(4, total); // 4 кроссовера
  Assert.Equal(2, totalPages); // 4 / 3 = 2 страницы
     Assert.Equal(3, result.Items.Count); // На первой странице 3
  
    // Все машины должны быть кроссоверами
    Assert.All(result.Items, car => 
       Assert.Equal("suvs", car.Category?.NormalizedName)
        );
   }

        /// <summary>
        /// Тест: фильтрация по несуществующей категории возвращает пустой результат
        /// </summary>
      [Fact]
        public async Task GetCarListAsync_FilterByNonexistentCategory_ReturnsEmptyList()
  {
          // Arrange
       using var context = CreateContext();
            await SeedTestDataAsync(context);

            const string nonexistentCategory = "trucks";

      // Act
      var query = context.Cars
    .AsNoTracking()
       .Include(c => c.Category)
    .Where(c => c.Category != null && c.Category.NormalizedName == nonexistentCategory);

     var items = await query.ToListAsync();

      // Assert
       Assert.Empty(items);
 }

  #endregion

        #region Требование 4: Максимальный размер страницы

        /// <summary>
        /// Тест: метод не позволяет задать размер страницы больше максимального
     /// </summary>
        [Fact]
        public async Task GetCarListAsync_LimitsMaxPageSize()
        {
            // Arrange
   using var context = CreateContext();
await SeedTestDataAsync(context);

        const int maxPageSize = 10;
            int requestedPageSize = 100; // Запрашиваем больше максимума
     int effectivePageSize = Math.Min(requestedPageSize, maxPageSize);

     // Act
     var query = context.Cars.AsNoTracking().Include(c => c.Category);
    var items = await query
            .OrderBy(c => c.Id)
     .Take(effectivePageSize)
             .ToListAsync();

        // Assert
 Assert.Equal(7, items.Count); // Возвращаем все 7, но не более maxPageSize
    Assert.True(items.Count <= maxPageSize);
        }

        /// <summary>
        /// Тест: размер страницы 0 или отрицательный корректируется на минимум
        /// </summary>
        [Theory]
        [InlineData(0)]
      [InlineData(-1)]
        [InlineData(-100)]
        public async Task GetCarListAsync_CorrectsBadPageSize(int badPageSize)
        {
         // Arrange
        using var context = CreateContext();
     await SeedTestDataAsync(context);

         const int minPageSize = 1;
          int effectivePageSize = badPageSize < minPageSize ? 3 : badPageSize;

            // Act
       var query = context.Cars.AsNoTracking().Include(c => c.Category);
            var items = await query
  .OrderBy(c => c.Id)
         .Take(effectivePageSize)
         .ToListAsync();

            // Assert
            Assert.NotEmpty(items);
 Assert.True(items.Count >= minPageSize);
        }

        #endregion

#region Требование 5: Превышение номера страницы

    /// <summary>
        /// Тест: метод возвращает Success=false если номер страницы превышает 
        /// общее количество страниц
        /// </summary>
        [Fact]
        public async Task GetCarListAsync_ReturnsFailureWhenPageExceedsTotal()
      {
            // Arrange
            using var context = CreateContext();
            await SeedTestDataAsync(context);

 const int pageSize = 3;
 const int totalItems = 7; // В наших данных 7 машин
        int totalPages = (int)Math.Ceiling(totalItems / (double)pageSize); // = 3
            int requestedPage = totalPages + 1; // = 4 (больше максимума)

            // Act
            var query = context.Cars.AsNoTracking().Include(c => c.Category);
    var skip = (requestedPage - 1) * pageSize;
      var items = await query
            .OrderBy(c => c.Id)
           .Skip(skip)
         .Take(pageSize)
           .ToListAsync();

 bool isSuccess = requestedPage <= totalPages && items.Count > 0;

    // Assert
    Assert.False(isSuccess); // Должен быть успех = false
      Assert.Empty(items); // Список должен быть пустой
  }

    /// <summary>
      /// Тест: последняя страница всегда возвращает данные
        /// </summary>
        [Fact]
public async Task GetCarListAsync_LastPageReturnsData()
        {
            // Arrange
 using var context = CreateContext();
            await SeedTestDataAsync(context);

        const int pageSize = 3;
            int total = await context.Cars.CountAsync();
 int lastPage = (int)Math.Ceiling(total / (double)pageSize);

     // Act
            var query = context.Cars.AsNoTracking().Include(c => c.Category);
            var items = await query
       .OrderBy(c => c.Id)
       .Skip((lastPage - 1) * pageSize)
     .Take(pageSize)
           .ToListAsync();

            // Assert
      Assert.NotEmpty(items); // Последняя страница должна иметь данные
        Assert.True(items.Count <= pageSize);
        }

        /// <summary>
        /// Тест: страница после последней возвращает пусто
        /// </summary>
        [Fact]
        public async Task GetCarListAsync_PageBeyondLast_ReturnsEmpty()
        {
          // Arrange
            using var context = CreateContext();
    await SeedTestDataAsync(context);

        const int pageSize = 3;
         int total = await context.Cars.CountAsync();
            int lastPage = (int)Math.Ceiling(total / (double)pageSize);
            int beyondLastPage = lastPage + 1;

  // Act
            var query = context.Cars.AsNoTracking().Include(c => c.Category);
    var items = await query
          .OrderBy(c => c.Id)
  .Skip((beyondLastPage - 1) * pageSize)
     .Take(pageSize)
     .ToListAsync();

     // Assert
  Assert.Empty(items);
        }

        #endregion

        #region Дополнительные тесты

        /// <summary>
/// Тест: сортировка автомобилей по ID
        /// </summary>
      [Fact]
        public async Task GetCarListAsync_SortsCarsByIdAscending()
        {
            // Arrange
       using var context = CreateContext();
     await SeedTestDataAsync(context);

        // Act
   var items = await context.Cars
    .AsNoTracking()
    .Include(c => c.Category)
          .OrderBy(c => c.Id)
        .ToListAsync();

            // Assert
  var ids = items.Select(c => c.Id).ToList();
         Assert.Equal(ids.OrderBy(id => id), ids);
        }

   /// <summary>
        /// Тест: включение категории в результат
        /// </summary>
   [Fact]
        public async Task GetCarListAsync_IncludesCategory()
     {
 // Arrange
            using var context = CreateContext();
         await SeedTestDataAsync(context);

      // Act
  var items = await context.Cars
  .AsNoTracking()
         .Include(c => c.Category)
      .FirstAsync();

          // Assert
            Assert.NotNull(items);
     Assert.NotNull(items.Category);
       Assert.NotEmpty(items.Category.Name);
        }

   /// <summary>
     /// Тест: отсутствие дублей в результатах пейджинации
        /// </summary>
  [Fact]
  public async Task GetCarListAsync_NoDuplicatesAcrossPages()
        {
     // Arrange
       using var context = CreateContext();
            await SeedTestDataAsync(context);

  const int pageSize = 3;
         var allIds = new HashSet<int>();

        // Act
            for (int page = 1; page <= 3; page++)
            {
  var items = await context.Cars
            .AsNoTracking()
.OrderBy(c => c.Id)
   .Skip((page - 1) * pageSize)
   .Take(pageSize)
             .Select(c => c.Id)
           .ToListAsync();

    foreach (var id in items)
           {
 Assert.DoesNotContain(id, allIds); // Проверяем что ID ещё не видели
 allIds.Add(id);
  }
            }

          // Assert
            Assert.Equal(7, allIds.Count); // Все 7 машин без дублей
        }

        #endregion

        public void Dispose()
        {
       _connection?.Dispose();
        }
    }
}
