using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Lab1.Services.CarService;
using CarEntity = Lab1.Domain.Entities.Car;

namespace Lab1.Areas.Admin.Pages.Car
{
    public class DetailsModel : PageModel
    {
        private readonly ICarService _service;
        public DetailsModel(ICarService service) { _service = service; }

        public CarEntity Car { get; set; } = default!;

        public async Task<IActionResult> OnGetAsync(int? id)
        {
            if (id == null) return NotFound();

            var resp = await _service.GetCarByIdAsync(id.Value);
            var car = resp.Data;      // <— распаковка
            if (car == null) return NotFound();

            Car = car;
            return Page();
        }
    }
}
