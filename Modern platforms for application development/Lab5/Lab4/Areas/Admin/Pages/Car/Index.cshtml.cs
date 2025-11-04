using Lab1.Services.CarService;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using CarEntity = Lab1.Domain.Entities.Car;

namespace Lab1.Areas.Admin.Pages.Car;

public class IndexModel : PageModel
{
    private readonly ICarService _service;
    public IndexModel(ICarService service) { _service = service; }

    public IList<CarEntity> Cars { get; set; } = new List<CarEntity>();

    [BindProperty(SupportsGet = true)]
    public int PageNo { get; set; } = 1;

    public async Task OnGetAsync()
    {
        if (PageNo < 1) PageNo = 1;
        var resp = await _service.GetCarListAsync(null, PageNo);
        Cars = resp.Data?.Items ?? new List<CarEntity>();
    }
}
