using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.AspNetCore.Mvc.Rendering;
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using CarEntity = Lab1.Domain.Entities.Car;
using CategoryEntity = Lab1.Domain.Entities.Category;

namespace Lab1.Areas.Admin.Pages.Car;

public class CreateModel : PageModel
{
    private readonly ICarService _service;
    private readonly ICategoryService _categoryService;

    public CreateModel(ICarService service, ICategoryService categoryService)
    {
        _service = service;
        _categoryService = categoryService;
    }

    [BindProperty] public CarEntity Car { get; set; } = new();
    [BindProperty] public IFormFile? Image { get; set; }

    // выпадающий список категорий
    public List<SelectListItem> CategoryItems { get; set; } = new();

    // выбранная категория (Id)
    [BindProperty] public int? SelectedCategoryId { get; set; }

    // текст ошибки для показа на форме (если что-то пошло не так)
    public string? ErrorText { get; set; }

    public async Task<IActionResult> OnGet()
    {
        await LoadCategoriesAsync();
        return Page();
    }

    public async Task<IActionResult> OnPostAsync()
    {
        if (!ModelState.IsValid)
        {
            await LoadCategoriesAsync();
            return Page();
        }

        // проставляем категорию по выбранному Id
        Car.Category = SelectedCategoryId.HasValue
            ? new CategoryEntity { Id = SelectedCategoryId.Value }
            : null;

        var resp = await _service.CreateCarAsync(Car, Image);

        if (resp?.Data == null)
        {
            ErrorText = resp?.ErrorMessage ?? "Create failed";
            ModelState.AddModelError(string.Empty, ErrorText);
            await LoadCategoriesAsync();
            return Page();
        }

        // после создания — на список (можешь сменить на Details)
        return RedirectToPage("./Index");
    }

    private async Task LoadCategoriesAsync()
    {
        var resp = await _categoryService.GetCategoryListAsync();
        var list = resp.Data ?? new List<CategoryEntity>();
        CategoryItems = list
            .Select(c => new SelectListItem { Value = c.Id.ToString(), Text = c.Name })
            .ToList();
    }
}
