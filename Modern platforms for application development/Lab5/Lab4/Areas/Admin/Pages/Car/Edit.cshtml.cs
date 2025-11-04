using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.AspNetCore.Mvc.Rendering;
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using CarEntity = Lab1.Domain.Entities.Car;
using CategoryEntity = Lab1.Domain.Entities.Category;

namespace Lab1.Areas.Admin.Pages.Car;

public class EditModel : PageModel
{
    private readonly ICarService _service;
    private readonly ICategoryService _categoryService;

    public EditModel(ICarService service, ICategoryService categoryService)
    {
        _service = service;
        _categoryService = categoryService;
    }

    [BindProperty] public CarEntity Car { get; set; } = new();
    [BindProperty] public IFormFile? Image { get; set; }

    public List<SelectListItem> CategoryItems { get; set; } = new();
    [BindProperty] public int? SelectedCategoryId { get; set; }

    public string? ErrorText { get; set; }

    public async Task<IActionResult> OnGetAsync(int? id)
    {
        if (id == null) return RedirectToPage("./Index");

        var resp = await _service.GetCarByIdAsync(id.Value);
        var car = resp.Data;
        if (car == null)
        {
            ErrorText = resp.ErrorMessage ?? "Not found";
            return RedirectToPage("./Index");
        }

        Car = car;
        await LoadCategoriesAsync();
        SelectedCategoryId = Car.Category?.Id;

        return Page();
    }

    public async Task<IActionResult> OnPostAsync()
    {
        if (!ModelState.IsValid)
        {
            await LoadCategoriesAsync();
            return Page();
        }

        Car.Category = SelectedCategoryId.HasValue
            ? new CategoryEntity { Id = SelectedCategoryId.Value }
            : null;

        await _service.UpdateCarAsync(Car.Id, Car, Image);
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
