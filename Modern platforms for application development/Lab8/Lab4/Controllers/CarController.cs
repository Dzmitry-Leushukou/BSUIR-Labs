using Microsoft.AspNetCore.Mvc;
using System.Linq;
using System.Threading.Tasks;
using Lab1.Extensions; // <-- если расширение в другом namespace (напр. Lab1.UI.Extensions) — замени

namespace Lab1.UI.Controllers
{
    [Route("Catalog")]
    public class CarController : Controller
    {
        private readonly ICarService _carService;
        private readonly ICategoryService _categoryService;
        private readonly IConfiguration _configuration;

        public CarController(
            ICarService carService,
            ICategoryService categoryService,
            IConfiguration configuration)
        {
            _carService = carService;
            _categoryService = categoryService;
            _configuration = configuration;
        }

        [HttpGet]
        [Route("")]
        [Route("{category?}")]
        public async Task<IActionResult> Index(string? category, int pageNo = 1)
        {
            var categoriesResponse = await _categoryService.GetCategoryListAsync();
            if (!categoriesResponse.Successfull)
                return NotFound("Categories could not be loaded.");

            ViewBag.Categories = categoriesResponse.Data;

            var productResponse = await _carService.GetCarListAsync(category, pageNo);
            if (!productResponse.Successfull)
                return NotFound(productResponse.ErrorMessage);

            // отображаемое имя категории
            var currentCategory = string.IsNullOrWhiteSpace(category)
                ? "Все Авто"
                : (categoriesResponse.Data.FirstOrDefault(c => c.NormalizedName == category)?.Name ?? "Все Авто");

            ViewBag.CurrentCategory = currentCategory;
            ViewBag.CurrentPage = pageNo;
            ViewBag.TotalPages = productResponse.Data.TotalPages;

            // Ajax-запрос? -> отдаем только список + пейджер
            if (Request.IsAjaxRequest())
                return PartialView("_ListPartial", productResponse.Data);

            // Обычный запрос -> полная страница
            return View(productResponse.Data);
        }
    }
}
