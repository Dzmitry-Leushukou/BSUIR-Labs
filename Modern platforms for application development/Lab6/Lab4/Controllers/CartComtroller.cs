using Microsoft.AspNetCore.Mvc;

namespace Lab1.UI.Controllers;

public class CartController : Controller
{
    public IActionResult Index()
    {
        return View();
    }
}