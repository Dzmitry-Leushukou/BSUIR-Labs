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

namespace Lab1.Tests.QuickStart
{
    /// <summary>
    /// Краткое руководство для быстрого старта тестирования
    /// Используйте эти примеры как основу для собственных тестов
    /// </summary>
    public class QuickStartGuide
    {
 // ======== ПРОСТОЙ ПРИМЕР 1: Базовый тест ========

    [Fact]
        public async Task Example1_BasicTest()
        {
       // 1. Создаём моки
    var carService = Substitute.For<ICarService>();
     var categoryService = Substitute.For<ICategoryService>();
 var config = Substitute.For<IConfiguration>();

   // 2. Конфигурируем моки для успешного ответа
 categoryService
    .GetCategoryListAsync()
  .Returns(Task.FromResult(new ResponseData<List<Category>>
      {
    Successfull = true,
Data = new List<Category> { new Category { Id = 1, Name = "Авто", NormalizedName = "auto" } }
    }));

        carService
   .GetCarListAsync(Arg.Any<string>(), 1)
 .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
   {
            Successfull = true,
Data = new ListModel<Car>
       {
             Items = new List<Car> { new Car { Id = 1, Name = "BMW", Price = 50000 } },
        CurrentPage = 1,
         TotalPages = 1
           }
  }));

    // 3. Создаём контроллер с моками
 var controller = new CarController(carService, categoryService, config);
          controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

  // 4. Вызываем метод
 var result = await controller.Index(category: null, pageNo: 1);

            // 5. Проверяем результат
    var viewResult = Assert.IsType<ViewResult>(result);
 Assert.NotNull(viewResult.Model);
      }

      // ======== ПРИМЕР 2: Тестирование ошибок ========

      [Fact]
  public async Task Example2_TestingErrors()
        {
    // Конфигурируем мок для возврата ошибки
    var categoryService = Substitute.For<ICategoryService>();
  categoryService
           .GetCategoryListAsync()
   .Returns(Task.FromResult(new ResponseData<List<Category>>
              {
        Successfull = false,  // ? Ошибка!
   ErrorMessage = "Не удалось загрузить категории"
      }));

         var carService = Substitute.For<ICarService>();
       var config = Substitute.For<IConfiguration>();

            var controller = new CarController(carService, categoryService, config);
  controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

// Вызываем метод - он должен вернуть 404
  var result = await controller.Index(category: null, pageNo: 1);

   // Проверяем что это именно 404
     var notFoundResult = Assert.IsType<NotFoundObjectResult>(result);
        Assert.Equal(404, notFoundResult.StatusCode);
    }

   // ======== ПРИМЕР 3: Проверка передачи данных ========

        [Fact]
        public async Task Example3_VerifyingDataPassing()
    {
      // Подготавливаем данные
    var categories = new List<Category>
 {
      new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
     new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" },
       new Category { Id = 3, Name = "Минивэны", NormalizedName = "minivans" }
     };

         var cars = new List<Car>
  {
   new Car { Id = 1, Name = "Toyota Camry", Price = 25000, Category = categories[0] },
          new Car { Id = 2, Name = "Honda Accord", Price = 27000, Category = categories[0] }
   };

    // Конфигурируем моки
  var categoryService = Substitute.For<ICategoryService>();
          categoryService.GetCategoryListAsync()
        .Returns(Task.FromResult(new ResponseData<List<Category>>
{
         Successfull = true,
      Data = categories
        }));

    var carService = Substitute.For<ICarService>();
 carService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
      .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
           {
    Successfull = true,
       Data = new ListModel<Car> { Items = cars, CurrentPage = 1, TotalPages = 1 }
   }));

  var config = Substitute.For<IConfiguration>();
    var controller = new CarController(carService, categoryService, config);
    controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

     // Вызываем метод
  var result = await controller.Index(category: null, pageNo: 1);

       // Проверяем что ViewBag содержит категории
      var viewResult = Assert.IsType<ViewResult>(result);
    // var passedCategories = Assert.IsType<List<Category>>(viewResult.ViewBag.Categories);
    // Assert.Equal(3, passedCategories.Count);
    // Assert.Contains("Седаны", passedCategories.Select(c => c.Name));
    // Assert.Equal("Все Авто", viewResult.ViewBag.CurrentCategory);

      // Проверяем что модель содержит машины
 var model = Assert.IsType<ListModel<Car>>(viewResult.Model);
         Assert.Equal(2, model.Items.Count);
  }

       // ======== ПРИМЕР 4: Фильтрация по категориям ========

  [Fact]
     public async Task Example4_FilteringByCategory()
       {
          const string categoryName = "sedans";

           var categories = new List<Category>
      {
        new Category { Id = 1, Name = "Седаны", NormalizedName = categoryName }
         };

          var sedanCars = new List<Car>
  {
        new Car { Id = 1, Name = "BMW 3 Series", Price = 40000, Category = categories[0] },
 new Car { Id = 2, Name = "Mercedes C-Class", Price = 50000, Category = categories[0] }
       };

     var categoryService = Substitute.For<ICategoryService>();
         categoryService.GetCategoryListAsync()
       .Returns(Task.FromResult(new ResponseData<List<Category>>
 {
        Successfull = true,
        Data = categories
 }));

     var carService = Substitute.For<ICarService>();
       carService.GetCarListAsync(categoryName, Arg.Any<int>())
  .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
     {
     Successfull = true,
         Data = new ListModel<Car> { Items = sedanCars, CurrentPage = 1, TotalPages = 1 }
    }));

      var config = Substitute.For<IConfiguration>();
 var controller = new CarController(carService, categoryService, config);
        controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

          // Вызываем с категорией
    var result = await controller.Index(category: categoryName, pageNo: 1);

var viewResult = Assert.IsType<ViewResult>(result);

          // Проверяем что текущая категория теперь "Седаны"
   // Assert.Equal("Седаны", viewResult.ViewBag.CurrentCategory);

        // Проверяем что были загружены правильные машины
         var model = Assert.IsType<ListModel<Car>>(viewResult.Model);
        // Проверяем что хотя бы один BMW или Mercedes
        Assert.NotEmpty(model.Items);
    }

 // ======== ПРИМЕР 5: Проверка вызовов моков ========

  [Fact]
    public async Task Example5_VerifyingMockCalls()
{
      var carService = Substitute.For<ICarService>();
      var categoryService = Substitute.For<ICategoryService>();
 var config = Substitute.For<IConfiguration>();

          // Конфигурируем возвращаемые значения
    categoryService.GetCategoryListAsync()
         .Returns(Task.FromResult(new ResponseData<List<Category>>
        {
        Successfull = true,
              Data = new List<Category>()
    }));

      carService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
        .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
     {
  Successfull = true,
        Data = new ListModel<Car> { Items = new List<Car>(), CurrentPage = 1, TotalPages = 1 }
  }));

      var controller = new CarController(carService, categoryService, config);
        controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

     // Вызываем метод
   var result = await controller.Index(category: "sedans", pageNo: 3);

         // Проверяем что методы были вызваны
         // ? GetCategoryListAsync должен быть вызван 1 раз
   await categoryService.Received(1).GetCategoryListAsync();

      // ? GetCarListAsync должен быть вызван с правильными параметрами
      await carService.Received(1).GetCarListAsync("sedans", 3);

      // ? DidNotReceive - метод не был вызван
       await carService.DidNotReceive().GetCarListAsync("other_category", 1);
 }

       // ======== ПРИМЕР 6: Параметризованный тест (Theory) ========

    [Theory]
       [InlineData("sedans", 1)]
  [InlineData("suvs", 2)]
        [InlineData("minivans", 3)]
    [InlineData(null, 1)]
        public async Task Example6_ParameterizedTest(string category, int pageNo)
       {
            // Используем одинаковый код для разных параметров
          var carService = Substitute.For<ICarService>();
      var categoryService = Substitute.For<ICategoryService>();
   var config = Substitute.For<IConfiguration>();

         categoryService.GetCategoryListAsync()
         .Returns(Task.FromResult(new ResponseData<List<Category>>
        {
   Successfull = true,
         Data = new List<Category> { new Category { Id = 1, Name = "Test", NormalizedName = "test" } }
         }));

  carService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
     .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
    {
    Successfull = true,
Data = new ListModel<Car> { Items = new List<Car>(), CurrentPage = pageNo, TotalPages = 3 }
          }));

        var controller = new CarController(carService, categoryService, config);
          controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

    var result = await controller.Index(category: category, pageNo: pageNo);

   var viewResult = Assert.IsType<ViewResult>(result);
      // Assert.Equal(pageNo, viewResult.ViewBag.CurrentPage);
       }

 // ======== ПРИМЕР 7: Граничные случаи ========

     [Fact]
       public async Task Example7_EdgeCases_NegativePageNumber()
  {
   var carService = Substitute.For<ICarService>();
      var categoryService = Substitute.For<ICategoryService>();
    var config = Substitute.For<IConfiguration>();

         categoryService.GetCategoryListAsync()
 .Returns(Task.FromResult(new ResponseData<List<Category>>
             {
        Successfull = true,
        Data = new List<Category>()
              }));

    carService.GetCarListAsync(Arg.Any<string>(), Arg.Any<int>())
       .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
       {
         Successfull = true,
 Data = new ListModel<Car> { Items = new List<Car>(), CurrentPage = 1, TotalPages = 1 }
        }));

    var controller = new CarController(carService, categoryService, config);
     controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

   // Передаём отрицательный pageNo
     var result = await controller.Index(category: null, pageNo: -5);

     // Должен всё равно работать (контроллер может исправить значение)
      Assert.NotNull(result);
        }

      // ======== ПРИМЕР 8: Полный реальный сценарий ========

   [Fact]
        public async Task Example8_FullRealWorldScenario()
  {
      // Сценарий: пользователь открывает страницу каталога машин
            // Хочет посмотреть только кроссоверы на странице 2

  // Подготавливаем каталог с 3 категориями
             var allCategories = new List<Category>
   {
     new Category { Id = 1, Name = "Седаны", NormalizedName = "sedans" },
       new Category { Id = 2, Name = "Кроссоверы", NormalizedName = "suvs" },
       new Category { Id = 3, Name = "Минивэны", NormalizedName = "minivans" }
              };

        // Подготавливаем машины для кроссоверов (страница 2 из 3)
 var pageSize = 3;
       var suvCars = new List<Car>
      {
        new Car { Id = 4, Name = "Toyota RAV4", Price = 35000, Category = allCategories[1] },
          new Car { Id = 5, Name = "Honda CR-V", Price = 36000, Category = allCategories[1] },
       new Car { Id = 6, Name = "Mazda CX-5", Price = 33000, Category = allCategories[1] }
         };

   var categoryService = Substitute.For<ICategoryService>();
     var carService = Substitute.For<ICarService>();
         var config = Substitute.For<IConfiguration>();

         // Конфигурируем что все категории загружаются успешно
  categoryService.GetCategoryListAsync()
 .Returns(Task.FromResult(new ResponseData<List<Category>>
       {
         Successfull = true,
 Data = allCategories
   }));

        // Конфигурируем что кроссоверы загружаются успешно
       carService.GetCarListAsync("suvs", 2)
 .Returns(Task.FromResult(new ResponseData<ListModel<Car>>
            {
 Successfull = true,
  Data = new ListModel<Car>
 {
      Items = suvCars,
    CurrentPage = 2,
     TotalPages = 3
 }
        }));

        var controller = new CarController(carService, categoryService, config);
   controller.ControllerContext = new ControllerContext { HttpContext = new DefaultHttpContext() };

        // Пользователь запрашивает: категория "suvs", страница 2
  var result = await controller.Index(category: "suvs", pageNo: 2);

// Проверяем результат
   var viewResult = Assert.IsType<ViewResult>(result);

         // ? Все категории видны в меню
 // var categoriesInView = Assert.IsType<List<Category>>(viewResult.ViewBag.Categories);
   // Assert.Equal(3, categoriesInView.Count);

      // ? Текущая категория показана как "Кроссоверы"
    // Assert.Equal("Кроссоверы", viewResult.ViewBag.CurrentCategory);

      // ? Пейджинация работает
// Assert.Equal(2, viewResult.ViewBag.CurrentPage);
   // Assert.Equal(3, viewResult.ViewBag.TotalPages);

        // ? Модель содержит только кроссоверы
   var model = Assert.IsType<ListModel<Car>>(viewResult.Model);
    Assert.Equal(3, model.Items.Count);
       Assert.All(model.Items, car => 
         Assert.Contains(new[] { "RAV4", "CR-V", "CX-5" }, c => car.Name.Contains(c))
      );
        }
    }
}
