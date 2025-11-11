using Microsoft.AspNetCore.Mvc;

namespace Lab1.UI.Controllers;

public class AdminController : Controller
{
    public IActionResult Index()
    {
        return View();
    }
}