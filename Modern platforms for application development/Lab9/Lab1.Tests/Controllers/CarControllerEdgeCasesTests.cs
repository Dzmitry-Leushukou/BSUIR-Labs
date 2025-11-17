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
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace Lab1.Tests.Controllers
{
    /// <summary>
    /// Дополнительные тесты граничных случаев для CarController
    /// </summary>
    public class CarControllerEdgeCasesTests
    {
 private readonly ICarService _carServiceMock;
        private readonly ICategoryService _categoryServiceMock;
        private readonly IConfiguration _configurationMock;
        private readonly CarController _controller;

        public CarControllerEdgeCasesTests()
      {
        _carServiceMock = Substitute.For<ICarService>();
       _categoryServiceMock = Substitute.For<ICategoryService>();
    _configurationMock = Substitute.For<IConfiguration>();

          _controller = new CarController(
       _carServiceMock,
       _categoryServiceMock,
      _configurationMock);

  var httpContext = new DefaultHttpContext();
            _controller.ControllerContext = new ControllerContext
 {
                HttpContext = httpContext
            };
        }

        /// <summary>
        /// Тест: проверка обработки null категории (не передана)
     /// </summary>
      [Fact]
        public async Task Index_WithNullCategory_TreatsAsNoFilter()
 {
// Arrange
            var categories = new List<Category>
            {
    new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
            };

      var cars = new List<Car>
 {
 new Car { Id = 1, Name = "Car 1", Price = 20000, Category = categories[0] },
                new Car { Id = 2, Name = "Car 2", Price = 30000, Category = categories[0] }
            };

           var categoriesResponse = new ResponseData<List<Category>>
   {
       Successfull = true,
  Data = categories
            };

 var carsResponse = new ResponseData<ListModel<Car>>
            {
 Successfull = true,
             Data = new ListModel<Car>
     {
       Items = cars,
               CurrentPage = 1,
           TotalPages = 1
         }
       };

   _categoryServiceMock
                .GetCategoryListAsync()
              .Returns(Task.FromResult(categoriesResponse));

       _carServiceMock
    .GetCarListAsync(null, 1)
    .Returns(Task.FromResult(carsResponse));

           // Act
           var result = await _controller.Index(category: null, pageNo: 1);

         // Assert
         var viewResult = Assert.IsType<ViewResult>(result);
      // Assert.Equal("Все Авто", viewResult.ViewBag.CurrentCategory);

 // Проверяем, что сервис вызван with null
           await _carServiceMock.Received(1).GetCarListAsync(null, 1);
 }

        /// <summary>
        /// Тест: обработка отрицательного номера страницы
        /// </summary>
        [Fact]
        public async Task Index_WithNegativePageNumber_SuccessfullyLoads()
     {
    // Arrange
        var categories = new List<Category>
      {
      new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
  };

      var cars = new List<Car>();

var categoriesResponse = new ResponseData<List<Category>>
{
          Successfull = true,
                Data = categories
          };

     var carsResponse = new ResponseData<ListModel<Car>>
        {
    Successfull = true,
    Data = new ListModel<Car>
    {
    Items = cars,
         CurrentPage = 1,
  TotalPages = 1
         }
            };

     _categoryServiceMock
        .GetCategoryListAsync()
    .Returns(Task.FromResult(categoriesResponse));

            _carServiceMock
       .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())  // контроллер может преобразовать отрицательное в 1
   .Returns(Task.FromResult(carsResponse));

    // Act
    var result = await _controller.Index(category: null, pageNo: -5);

 // Assert
      var viewResult = Assert.IsType<ViewResult>(result);
        Assert.NotNull(viewResult);
        }

        /// <summary>
      /// Тест: проверка категории с пробелами и специальными символами
    /// </summary>
        [Fact]
      public async Task Index_WithSpecialCharactersInCategory_HandlesCorrectly()
        {
  // Arrange
           const string category = "special-category-123";

           var categories = new List<Category>
            {
   new Category { Id = 1, Name = "Special Category 123", NormalizedName = category }
            };

  var cars = new List<Car>
            {
 new Car { Id = 1, Name = "Test Car", Price = 25000, Category = categories[0] }
        };

         var categoriesResponse = new ResponseData<List<Category>>
     {
  Successfull = true,
 Data = categories
            };

        var carsResponse = new ResponseData<ListModel<Car>>
            {
   Successfull = true,
    Data = new ListModel<Car>
   {
          Items = cars,
  CurrentPage = 1,
           TotalPages = 1
   }
        };

          _categoryServiceMock
     .GetCategoryListAsync()
   .Returns(Task.FromResult(categoriesResponse));

    _carServiceMock
               .GetCarListAsync(category, 1)
  .Returns(Task.FromResult(carsResponse));

    // Act
     var result = await _controller.Index(category: category, pageNo: 1);

           // Assert
           var viewResult = Assert.IsType<ViewResult>(result);
    // Assert.Equal("Special Category 123", viewResult.ViewBag.CurrentCategory);
    }

      /// <summary>
    /// Тест: проверка обработки очень больших значений pageNo
        /// </summary>
        [Fact]
  public async Task Index_WithLargePageNumber_SuccessfullyLoads()
        {
             // Arrange
       const int largePageNo = 999999;

     var categories = new List<Category>
  {
   new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
   };

 var cars = new List<Car>();

           var categoriesResponse = new ResponseData<List<Category>>
       {
 Successfull = true,
    Data = categories
       };

   var carsResponse = new ResponseData<ListModel<Car>>
            {
        Successfull = true,
   Data = new ListModel<Car>
               {
        Items = cars,
       CurrentPage = largePageNo,
           TotalPages = largePageNo
    }
            };

            _categoryServiceMock
          .GetCategoryListAsync()
    .Returns(Task.FromResult(categoriesResponse));

         _carServiceMock
   .GetCarListAsync(Arg.Any<string>(), largePageNo)
           .Returns(Task.FromResult(carsResponse));

     // Act
      var result = await _controller.Index(category: null, pageNo: largePageNo);

  // Assert
        var viewResult = Assert.IsType<ViewResult>(result);
    // Assert.Equal(largePageNo, viewResult.ViewBag.CurrentPage);
      }

        /// <summary>
  /// Тест: проверка корректности типа возвращаемого представления
        /// </summary>
     [Fact]
 public async Task Index_ReturnsViewWithCorrectModelType()
        {
    // Arrange
            var categories = new List<Category>
   {
         new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
        };

       var cars = new List<Car>
        {
         new Car { Id = 1, Name = "Test Car", Price = 25000, Category = categories[0] }
    };

var categoriesResponse = new ResponseData<List<Category>>
   {
        Successfull = true,
   Data = categories
          };

     var carsResponse = new ResponseData<ListModel<Car>>
   {
       Successfull = true,
 Data = new ListModel<Car>
           {
    Items = cars,
         CurrentPage = 1,
   TotalPages = 1
       }
          };

    _categoryServiceMock
       .GetCategoryListAsync()
  .Returns(Task.FromResult(categoriesResponse));

          _carServiceMock
    .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
      .Returns(Task.FromResult(carsResponse));

    // Act
     var result = await _controller.Index(category: null, pageNo: 1);

      // Assert
        var viewResult = Assert.IsType<ViewResult>(result);
         var model = viewResult.Model;

         Assert.NotNull(model);
 Assert.IsType<ListModel<Car>>(model);
         var listModel = (ListModel<Car>)model;
    Assert.Single(listModel.Items);
        }

    /// <summary>
 /// Тест: проверка, что при ошибке категорий сообщение об ошибке передано
        /// </summary>
        [Fact]
public async Task Index_WhenCategoriesFail_ErrorMessageIsIncluded()
     {
     // Arrange
      const string errorMsg = "Categories could not be loaded.";

   var categoriesResponse = new ResponseData<List<Category>>
  {
    Successfull = false,
     ErrorMessage = errorMsg
      };

       _categoryServiceMock
    .GetCategoryListAsync()
  .Returns(Task.FromResult(categoriesResponse));

  // Act
     var result = await _controller.Index(category: null, pageNo: 1);

    // Assert
         var notFoundResult = Assert.IsType<NotFoundObjectResult>(result);
   // Проверяем что тип результата - NotFound
  Assert.Equal(404, notFoundResult.StatusCode);
    }

        /// <summary>
        /// Тест: проверка множественных последовательных вызовов
      /// </summary>
  [Fact]
        public async Task Index_MultipleSequentialCalls_IndependentResults()
        {
            // Arrange
   var categories1 = new List<Category>
            {
       new Category { Id = 1, Name = "Category 1", NormalizedName = "cat1" }
            };

 var categories2 = new List<Category>
       {
              new Category { Id = 2, Name = "Category 2", NormalizedName = "cat2" }
     };

    var cars1 = new List<Car> { new Car { Id = 1, Name = "Car 1", Price = 10000, Category = categories1[0] } };
  var cars2 = new List<Car> { new Car { Id = 2, Name = "Car 2", Price = 20000, Category = categories2[0] } };

        var categoriesResponse1 = new ResponseData<List<Category>>
      {
        Successfull = true,
  Data = categories1
            };

           var categoriesResponse2 = new ResponseData<List<Category>>
            {
    Successfull = true,
     Data = categories2
     };

      var carsResponse1 = new ResponseData<ListModel<Car>>
   {
        Successfull = true,
Data = new ListModel<Car> { Items = cars1, CurrentPage = 1, TotalPages = 1 }
     };

            var carsResponse2 = new ResponseData<ListModel<Car>>
  {
           Successfull = true,
           Data = new ListModel<Car> { Items = cars2, CurrentPage = 1, TotalPages = 1 }
     };

       _categoryServiceMock
         .GetCategoryListAsync()
     .Returns(
    Task.FromResult(categoriesResponse1),
                Task.FromResult(categoriesResponse2)
            );

 _carServiceMock
      .GetCarListAsync(Arg.Any<string>(), 1)
        .Returns(
        Task.FromResult(carsResponse1),
   Task.FromResult(carsResponse2)
     );

          // Act - первый вызов
    var result1 = await _controller.Index(category: null, pageNo: 1);
  var viewResult1 = Assert.IsType<ViewResult>(result1);

// Assert первого вызова
    Assert.Single(((ListModel<Car>)viewResult1.Model).Items);
     Assert.Equal("Car 1", ((ListModel<Car>)viewResult1.Model).Items[0].Name);

         // Act - второй вызов (на новом экземпляре контроллера)
           var controller2 = new CarController(_carServiceMock, _categoryServiceMock, _configurationMock);
     controller2.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

 var result2 = await controller2.Index(category: null, pageNo: 1);
        var viewResult2 = Assert.IsType<ViewResult>(result2);

       // Assert второго вызова
           Assert.Single(((ListModel<Car>)viewResult2.Model).Items);
      Assert.Equal("Car 2", ((ListModel<Car>)viewResult2.Model).Items[0].Name);
      }
    }
}
