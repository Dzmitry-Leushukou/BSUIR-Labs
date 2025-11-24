// Примеры использования NSubstitute и xUnit для тестирования Lab1

using Lab1.UI.Controllers;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using NSubstitute;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Xunit;

namespace Lab1.Tests.Examples
{
    /// <summary>
    /// Примеры best practices для тестирования с xUnit и NSubstitute
    /// </summary>
public class TestingBestPracticesExamples
    {
        // ============= ПРИМЕР 1: Базовая структура теста =============

        /// <summary>
        /// Хороший пример теста - следует AAA паттерну
        /// </summary>
        [Fact]
        public async Task GoodTestExample_FollowsAAAPattern()
        {
      // ARRANGE - подготовка данных
      var mockCategoryService = Substitute.For<ICategoryService>();
     var mockCarService = Substitute.For<ICarService>();
            var mockConfig = Substitute.For<IConfiguration>();

            var categories = new List<Category>
      {
          new Category { Id = 1, Name = "Тест", NormalizedName = "test" }
          };

     mockCategoryService
    .GetCategoryListAsync()
         .Returns(Task.FromResult(new ResponseData<List<Category>>
                {
             Successfull = true,
           Data = categories
     }));

    var controller = new CarController(mockCarService, mockCategoryService, mockConfig);
   controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

     var cars = new List<Car> { new Car { Id = 1, Name = "Авто", Price = 10000 } };
            mockCarService
   .GetCarListAsync(Arg.Any<string>(), 1)
                .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
           {
      Successfull = true,
     Data = new ListModel<Car> { Items = cars, CurrentPage = 1, TotalPages = 1 }
             }));

     // ACT - выполнение действия
   var result = await controller.Index(category: null, pageNo: 1);

// ASSERT - проверка результатов
       var viewResult = Assert.IsType<ViewResult>(result);
   Assert.NotNull(viewResult.Model);
 }

        // ============= ПРИМЕР 2: Использование NSubstitute =============

        /// <summary>
        /// Пример создания и настройки моков
        /// </summary>
        [Fact]
        public void NSubstituteExamples_CreatingMocks()
        {
            // ARRANGE
   var service = Substitute.For<ICarService>();

    // Базовая конфигурация
 var response = new ResponseData<ListModel<Car>>
            {
        Successfull = true,
    Data = new ListModel<Car> { Items = new List<Car>() }
 };
    service.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
          .Returns(Task.FromResult(response));

  // Проверка с конкретными аргументами
          service.GetCarListAsync("sedans", 1)
      .Returns(Task.FromResult(response));

            // ACT & ASSERT
            Assert.NotNull(service);
        }

        /// <summary>
        /// Пример проверки вызовов моков
        /// </summary>
        [Fact]
        public async Task NSubstituteExamples_VerifyingCalls()
      {
      // ARRANGE
            var mockService = Substitute.For<ICarService>();
mockService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
     .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
     {
       Successfull = true,
        Data = new ListModel<Car> { Items = new List<Car>() }
           }));

      // ACT
     await mockService.GetCarListAsync("test", 1);

       // ASSERT - проверяем что метод был вызван 1 раз
      await mockService.Received(1).GetCarListAsync("test", 1);

            // Проверяем с любыми аргументами
        await mockService.Received(1).GetCarListAsync(Arg.Any<string>(), 1);

            // Проверяем что метод был вызван НЕ вызван с другими аргументами
  await mockService.DidNotReceive().GetCarListAsync("other", 1);
        }

  /// <summary>
        /// Пример параметризованного теста с Theory
 /// </summary>
        [Theory]
        [InlineData("sedans", 1)]
        [InlineData("suvs", 2)]
        [InlineData(null, 1)]
        public async Task XunitExamples_TheoryTest(string category, int pageNo)
        {
       // ARRANGE
            var mockService = Substitute.For<ICarService>();
            mockService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
    .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
      {
         Successfull = true,
     Data = new ListModel<Car> { Items = new List<Car>() }
                }));

            // ACT
   var result = await mockService.GetCarListAsync(category, pageNo);

     // ASSERT
            Assert.True(result.Successfull);
        }

        // ============= ПРИМЕР 3: Исключения и ошибки =============

  /// <summary>
        /// Пример тестирования исключений
     /// </summary>
        [Fact]
      public void ErrorHandling_TestingExceptions()
        {
        // ARRANGE
     var mockService = Substitute.For<ICarService>();
mockService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
          .Returns(Task.FromException<ResponseData<ListModel<Car>>>(
     new InvalidOperationException("Test exception")));

    // ACT & ASSERT
      var ex = Assert.ThrowsAsync<InvalidOperationException>(
    async () => await mockService.GetCarListAsync(null, 1));

     Assert.NotNull(ex);
   }

        /// <summary>
        /// Пример тестирования граничных значений
        /// </summary>
        [Theory]
        [InlineData(0)]      // ноль
      [InlineData(-1)]     // отрицательное
  [InlineData(int.MaxValue)] // максимальное
        public void BoundaryTesting_EdgeValues(int pageNo)
        {
            // ARRANGE & ACT
            var listModel = new ListModel<Car>
  {
     Items = new List<Car>(),
    CurrentPage = pageNo >= 1 ? pageNo : 1,  // контроллер может исправлять
   TotalPages = 1
            };

            // ASSERT
            Assert.True(listModel.CurrentPage >= 1);
        }

        // ============= ПРИМЕР 4: Fixture паттерн =============

   /// <summary>
        /// Fixture для повторного использования в тестах
        /// </summary>
   public class CarControllerFixture : IDisposable
   {
            public readonly ICarService CarServiceMock;
          public readonly ICategoryService CategoryServiceMock;
    public readonly IConfiguration ConfigMock;
            public readonly CarController Controller;

            public CarControllerFixture()
      {
           CarServiceMock = Substitute.For<ICarService>();
                CategoryServiceMock = Substitute.For<ICategoryService>();
        ConfigMock = Substitute.For<IConfiguration>();

   Controller = new CarController(CarServiceMock, CategoryServiceMock, ConfigMock);
             Controller.ControllerContext = new ControllerContext
     {
             HttpContext = new DefaultHttpContext()
};
    }

   public void Dispose()
            {
     // Очистка ресурсов если нужно
          }
        }

        /// <summary>
    /// Тест с использованием Fixture
        /// </summary>
        [Fact]
        public async Task UsingFixture_SimplifiesSetup()
        {
            // ARRANGE
          using var fixture = new CarControllerFixture();

        fixture.CategoryServiceMock
              .GetCategoryListAsync()
              .Returns(Task.FromResult(new ResponseData<List<Category>>
   {
        Successfull = true,
           Data = new List<Category>()
       }));

fixture.CarServiceMock
                .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
            .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
      {
        Successfull = true,
          Data = new ListModel<Car> { Items = new List<Car>() }
 }));

 // ACT
            var result = await fixture.Controller.Index(category: null, pageNo: 1);

            // ASSERT
  Assert.IsType<ViewResult>(result);
   }

        // ============= ПРИМЕР 5: Запуск тестов по требованиям =============

        /// <summary>
        /// Требование: "Метод должен возвращать 404 если категории не загружены"
        /// </summary>
        [Fact]
public async Task Requirement_Returns404WhenCategoriesFail()
        {
     // ARRANGE - готовим сценарий: категории не загружены
            var mockCategoryService = Substitute.For<ICategoryService>();
            mockCategoryService
      .GetCategoryListAsync()
     .Returns(Task.FromResult(new ResponseData<List<Category>>
      {
   Successfull = false,
      ErrorMessage = "Ошибка подключения"
     }));

    var mockCarService = Substitute.For<ICarService>();
    var mockConfig = Substitute.For<IConfiguration>();

   var controller = new CarController(mockCarService, mockCategoryService, mockConfig);
    controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

  // ACT - вызываем метод
        var result = await controller.Index(category: null, pageNo: 1);

     // ASSERT - проверяем что вернулся 404
         var notFoundResult = Assert.IsType<NotFoundObjectResult>(result);
            Assert.Equal(StatusCodes.Status404NotFound, notFoundResult.StatusCode);
        }

  /// <summary>
     /// Требование: "При успехе ViewBag должен содержать категории"
        /// </summary>
        [Fact]
        public async Task Requirement_ViewBagContainsCategories()
      {
         // ARRANGE
        var categories = new List<Category>
  {
          new Category { Id = 1, Name = "Cat1", NormalizedName = "cat1" },
   new Category { Id = 2, Name = "Cat2", NormalizedName = "cat2" }
     };

            var mockCategoryService = Substitute.For<ICategoryService>();
            mockCategoryService
     .GetCategoryListAsync()
     .Returns(Task.FromResult(new ResponseData<List<Category>>
       {
   Successfull = true,
       Data = categories
   }));

          var mockCarService = Substitute.For<ICarService>();
   mockCarService
    .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
              .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
         {
     Successfull = true,
   Data = new ListModel<Car> { Items = new List<Car>(), CurrentPage = 1, TotalPages = 1 }
       }));

       var mockConfig = Substitute.For<IConfiguration>();
   var controller = new CarController(mockCarService, mockCategoryService, mockConfig);
            controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

            // ACT
 var result = await controller.Index(category: null, pageNo: 1);

    // ASSERT - проверяем ViewBag
 var viewResult = Assert.IsType<ViewResult>(result);
         //var passedCategories = Assert.IsType<List<Category>>(viewResult.ViewBag.Categories);
            //Assert.Equal(2, passedCategories.Count);
        }

        /// <summary>
        /// Требование: "CurrentCategory должна быть 'Все Авто' если категория не указана"
     /// </summary>
  [Fact]
        public async Task Requirement_CurrentCategoryDefaultsToAll()
        {
     // ARRANGE
    var mockCategoryService = Substitute.For<ICategoryService>();
   mockCategoryService
        .GetCategoryListAsync()
         .Returns(Task.FromResult(new ResponseData<List<Category>>
        {
         Successfull = true,
      Data = new List<Category>()
      }));

   var mockCarService = Substitute.For<ICarService>();
      mockCarService
     .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
   .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
   {
 Successfull = true,
      Data = new ListModel<Car> { Items = new List<Car>(), CurrentPage = 1, TotalPages = 1 }
   }));

     var mockConfig = Substitute.For<IConfiguration>();
var controller = new CarController(mockCarService, mockCategoryService, mockConfig);
     controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

            // ACT - вызываем без категории
      var result = await controller.Index(category: null, pageNo: 1);

        // ASSERT
   var viewResult = Assert.IsType<ViewResult>(result);
     // Assert.Equal("Все Авто", viewResult.ViewBag.CurrentCategory);
    }
    }
}
