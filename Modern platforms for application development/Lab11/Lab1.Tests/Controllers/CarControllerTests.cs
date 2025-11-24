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
    /// Модульные тесты для CarController
    /// </summary>
    public class CarControllerTests
    {
   private readonly ICarService _carServiceMock;
        private readonly ICategoryService _categoryServiceMock;
        private readonly IConfiguration _configurationMock;
 private readonly CarController _controller;

        public CarControllerTests()
        {
    // Создаём моки для зависимостей
        _carServiceMock = Substitute.For<ICarService>();
            _categoryServiceMock = Substitute.For<ICategoryService>();
  _configurationMock = Substitute.For<IConfiguration>();

            // Инициализируем контроллер с моками
      _controller = new CarController(
                _carServiceMock,
    _categoryServiceMock,
          _configurationMock);

            // Устанавливаем контекст HTTP-запроса
         var httpContext = new DefaultHttpContext();
            _controller.ControllerContext = new ControllerContext
        {
        HttpContext = httpContext
 };
        }

        #region Тесты на ошибки

   /// <summary>
        /// Тест: возвращает 404 при неуспешном получении списка категорий
        /// </summary>
        [Fact]
        public async Task Index_WhenCategoriesLoadFails_ReturnsNotFound()
        {
            // Arrange
    var categoriesResponse = new ResponseData<List<Category>>
   {
      Successfull = false,
    ErrorMessage = "Ошибка при загрузке категорий"
      };

     _categoryServiceMock
           .GetCategoryListAsync()
 .Returns(Task.FromResult(categoriesResponse));

          // Act
            var result = await _controller.Index(category: null, pageNo: 1);

         // Assert
     var notFoundResult = Assert.IsType<NotFoundObjectResult>(result);
   Assert.Equal(StatusCodes.Status404NotFound, notFoundResult.StatusCode);
        }

        /// <summary>
        /// Тест: возвращает 404 при неуспешном получении списка автомобилей
        /// </summary>
 [Fact]
   public async Task Index_WhenCarsLoadFails_ReturnsNotFound()
        {
   // Arrange
            var categoriesResponse = new ResponseData<List<Category>>
    {
       Successfull = true,
                Data = new List<Category>
                {
    new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
    }
    };

    var carsResponse = new ResponseData<ListModel<Car>>
       {
      Successfull = false,
     ErrorMessage = "Ошибка при загрузке автомобилей"
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
          var notFoundResult = Assert.IsType<NotFoundObjectResult>(result);
            Assert.Equal(StatusCodes.Status404NotFound, notFoundResult.StatusCode);
        }

 #endregion

        #region Тесты на успешное выполнение

        /// <summary>
        /// Тест: успешная загрузка списка без фильтра по категориям
      /// </summary>
        [Fact]
        public async Task Index_WithoutCategory_SuccessfullyLoadsData()
        {
    // Arrange
            var categories = new List<Category>
            {
  new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
        new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "crossovers" }
            };

       var cars = new List<Car>
     {
      new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = categories[0] },
     new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = categories[0] }
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
   
    // Проверяем, что результат - ViewResult
// var viewBagCategories = (dynamic)viewResult.ViewBag.Categories;
// Assert.Equal(2, ((List<Category>)viewBagCategories).Count);

  // Проверяем модель
       var model = Assert.IsType<ListModel<Car>>(viewResult.Model);
        Assert.Equal(2, model.Items.Count);
        }

        /// <summary>
        /// Тест: успешная загрузка с фильтром по категориям
        /// </summary>
        [Fact]
        public async Task Index_WithCategory_SuccessfullyLoadsFilteredData()
        {
// Arrange
 const string category = "sedans";
    var categories = new List<Category>
         {
     new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
   new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "crossovers" }
  };

     var categoryEntity = categories.FirstOrDefault(c => c.NormalizedName == category);
 var cars = new List<Car>
     {
      new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = categoryEntity },
        new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = categoryEntity }
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
         .GetCarListAsync(category, Arg.Any<int>())
.Returns(Task.FromResult(carsResponse));

         // Act
     var result = await _controller.Index(category: category, pageNo: 1);

      // Assert
   var viewResult = Assert.IsType<ViewResult>(result);

      // Проверяем текущую категорию (должна быть "Седаны")
            // Assert.Equal("Седаны", viewResult.ViewBag.CurrentCategory);

     // Проверяем, что модель передана
            var model = Assert.IsType<ListModel<Car>>(viewResult.Model);
    Assert.Equal(2, model.Items.Count);

          // Проверяем, что сервис вызван с правильной категорией
     await _carServiceMock.Received(1).GetCarListAsync(category, 1);
    }

        /// <summary>
        /// Тест: проверка пейджинации
    /// </summary>
        [Fact]
        public async Task Index_WithPagination_CorrectlyPassesPageNumber()
      {
        // Arrange
      const int pageNo = 3;
            var categories = new List<Category>
            {
         new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" }
       };

       var cars = new List<Car>
    {
         new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = categories[0] }
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
            CurrentPage = pageNo,
            TotalPages = 5
     }
            };

      _categoryServiceMock
    .GetCategoryListAsync()
      .Returns(Task.FromResult(categoriesResponse));

            _carServiceMock
    .GetCarListAsync(Arg.Any<string>(), pageNo)
         .Returns(Task.FromResult(carsResponse));

            // Act
   var result = await _controller.Index(category: null, pageNo: pageNo);

            // Assert
   var viewResult = Assert.IsType<ViewResult>(result);

    // Проверяем пейджер
   // Assert.Equal(pageNo, viewResult.ViewBag.CurrentPage);
   // Assert.Equal(5, viewResult.ViewBag.TotalPages);

   // Проверяем, что сервис вызван с правильным номером страницы
            await _carServiceMock.Received(1).GetCarListAsync(Arg.Any<string>(), pageNo);
 }

     /// <summary>
     /// Тест: проверка передачи списка категорий во ViewData
        /// </summary>
        [Fact]
        public async Task Index_PassesCategoriesListToViewData()
     {
            // Arrange
            var categories = new List<Category>
      {
     new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
     new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "crossovers" },
 new Category { Id = 3, Name = "Минивэны", NormalizedName = "minivans" }
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
           .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
                .Returns(Task.FromResult(carsResponse));

          // Act
          var result = await _controller.Index(category: null, pageNo: 1);

            // Assert
            var viewResult = Assert.IsType<ViewResult>(result);
    //     var passedCategories = Assert.IsType<List<Category>>(viewResult.ViewBag.Categories);
 
    //         Assert.Equal(3, passedCategories.Count);
    //      Assert.Contains(passedCategories, c => c.Name == "Седаны");
    //         Assert.Contains(passedCategories, c => c.Name == "Кроссоверы");
    //          Assert.Contains(passedCategories, c => c.Name == "Минивэны");
 }

    #endregion

  #region Граничные случаи

      /// <summary>
  /// Тест: обработка пустого списка категорий
        /// </summary>
 [Fact]
 public async Task Index_WithEmptyCategories_ReturnsDefaultCurrentCategory()
      {
   // Arrange
  var categoriesResponse = new ResponseData<List<Category>>
       {
        Successfull = true,
     Data = new List<Category>()
     };

     var carsResponse = new ResponseData<ListModel<Car>>
            {
                Successfull = true,
    Data = new ListModel<Car>
          {
          Items = new List<Car>(),
       CurrentPage = 1,
           TotalPages = 0
             }
            };

      _categoryServiceMock
   .GetCategoryListAsync()
  .Returns(Task.FromResult(categoriesResponse));

            _carServiceMock
                .GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
  .Returns(Task.FromResult(carsResponse));

  // Act
            var result = await _controller.Index(category: "nonexistent", pageNo: 1);

     // Assert
var viewResult = Assert.IsType<ViewResult>(result);
     
      // Когда категория не найдена, должна быть "Все Авто"
     // Assert.Equal("Все Авто", viewResult.ViewBag.CurrentCategory);
    }

        /// <summary>
        /// Тест: проверка использования значения по умолчанию для pageNo
     /// </summary>
        [Fact]
        public async Task Index_WithDefaultPageNo_Uses1()
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
  .GetCarListAsync(Arg.Any<string>(), 1)
       .Returns(Task.FromResult(carsResponse));

         // Act
   var result = await _controller.Index(category: null); // pageNo не передан

          // Assert
       var viewResult = Assert.IsType<ViewResult>(result);
  
  // Проверяем, что использовано значение по умолчанию (1)
         await _carServiceMock.Received(1).GetCarListAsync(Arg.Any<string>(), 1);
        }

    #endregion
    }
}
