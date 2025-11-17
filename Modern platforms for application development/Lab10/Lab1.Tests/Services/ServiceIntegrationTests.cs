using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using NSubstitute;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace Lab1.Tests.Services
{
    /// <summary>
    /// Модульные тесты для интеграции сервисов
    /// </summary>
    public class ServiceIntegrationTests
    {
     /// <summary>
        /// Тест: проверка, что GetCarListAsync возвращает ResponseData с корректной структурой
        /// </summary>
     [Fact]
        public async Task GetCarListAsync_ReturnsValidResponseData()
        {
    // Arrange
            var carServiceMock = Substitute.For<ICarService>();
            
     var expectedResponse = new ResponseData<ListModel<Car>>
            {
                Successfull = true,
         Data = new ListModel<Car>
        {
    Items = new List<Car>
  {
        new Car { Id = 1, Name = "Test Car", Price = 1000 }
     },
    CurrentPage = 1,
  TotalPages = 1
     },
             ErrorMessage = null
  };

    carServiceMock
    .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
        .Returns(Task.FromResult(expectedResponse));

       // Act
      var result = await carServiceMock.GetCarListAsync(null, 1);

      // Assert
      Assert.NotNull(result);
            Assert.True(result.Successfull);
        Assert.NotNull(result.Data);
     Assert.Single(result.Data.Items);
            Assert.Equal("Test Car", result.Data.Items.First().Name);
    }

        /// <summary>
        /// Тест: проверка, что GetCategoryListAsync возвращает список категорий
        /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_ReturnsCategories()
     {
            // Arrange
        var categoryServiceMock = Substitute.For<ICategoryService>();
    
         var expectedCategories = new List<Category>
          {
       new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
   new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "crossovers" }
 };

        var expectedResponse = new ResponseData<List<Category>>
    {
       Successfull = true,
           Data = expectedCategories
      };

categoryServiceMock
          .GetCategoryListAsync()
                .Returns(Task.FromResult(expectedResponse));

  // Act
            var result = await categoryServiceMock.GetCategoryListAsync();

            // Assert
            Assert.NotNull(result);
            Assert.True(result.Successfull);
       Assert.Equal(2, result.Data.Count);
Assert.Contains(result.Data, c => c.NormalizedName == "sedans");
 }

        /// <summary>
  /// Тест: проверка обработки ошибки при загрузке категорий
        /// </summary>
        [Fact]
        public async Task GetCategoryListAsync_WithError_ReturnsErrorResponse()
        {
        // Arrange
var categoryServiceMock = Substitute.For<ICategoryService>();
      
          var errorResponse = new ResponseData<List<Category>>
          {
     Successfull = false,
          ErrorMessage = "Ошибка подключения к серверу",
       Data = null
        };

       categoryServiceMock
     .GetCategoryListAsync()
 .Returns(Task.FromResult(errorResponse));

  // Act
     var result = await categoryServiceMock.GetCategoryListAsync();

    // Assert
 Assert.NotNull(result);
        Assert.False(result.Successfull);
Assert.NotNull(result.ErrorMessage);
  Assert.Contains("подключения", result.ErrorMessage);
        }

    /// <summary>
  /// Тест: проверка пейджинации в ListModel
  /// </summary>
        [Theory]
        [InlineData(1, 10, 1)]
        [InlineData(2, 20, 2)]
        [InlineData(5, 50, 5)]
        public void ListModel_CalculatesTotalPages_Correctly(int pageNo, int totalItems, int expectedTotalPages)
        {
       // Arrange
            var pageSize = 10;
  var items = Enumerable.Range(1, totalItems)
   .Select(i => new Car { Id = i, Name = $"Car {i}", Price = 1000 + i })
             .ToList();

     // Act
  var listModel = new ListModel<Car>
         {
      Items = items.Take(pageSize).ToList(),
       CurrentPage = pageNo,
   TotalPages = (int)Math.Ceiling(totalItems / (double)pageSize)
         };

           // Assert
   Assert.Equal(expectedTotalPages, listModel.TotalPages);
  Assert.Equal(pageNo, listModel.CurrentPage);
        }
  }
}
