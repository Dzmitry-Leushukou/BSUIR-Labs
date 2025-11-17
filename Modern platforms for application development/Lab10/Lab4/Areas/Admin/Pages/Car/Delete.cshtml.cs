using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Lab1.Services.CarService;
using CarEntity = Lab1.Domain.Entities.Car;

namespace Lab1.Areas.Admin.Pages.Car
{
    public class DeleteModel : PageModel
    {
        private readonly ICarService _service;
        public DeleteModel(ICarService service) { _service = service; }

        [BindProperty] public CarEntity Car { get; set; } = default!;

        public async Task<IActionResult> OnGetAsync(int? id)
        {
            if (id == null) return NotFound();

            var resp = await _service.GetCarByIdAsync(id.Value);
            var car = resp.Data;      // <— распаковка
            if (car == null) return NotFound();

            Car = car;
            return Page();
        }

        public async Task<IActionResult> OnPostAsync(int? id)
        {
            if (id == null) return NotFound();
            await _service.DeleteCarAsync(id.Value);
            return RedirectToPage("./Index");
        }
    }
}
