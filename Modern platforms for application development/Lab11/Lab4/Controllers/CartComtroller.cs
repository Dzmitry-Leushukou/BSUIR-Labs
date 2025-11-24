using Microsoft.AspNetCore.Mvc;
using Lab1.Domain.Models;
using Lab1.Services.CarService;
using Microsoft.AspNetCore.Authorization;

namespace Lab1.UI.Controllers;

[Authorize]
[Route("[controller]")]
public class CartController : Controller
{
    private readonly ICarService _carService;
    private readonly Cart _cart;

    public CartController(ICarService carService, Cart cart)
    {
        _carService = carService;
      _cart = cart;
    }

    /// <summary>
    /// Добавить автомобиль в корзину
    /// </summary>
    [Route("add/{id:int}")]
    public async Task<IActionResult> Add(int id, string returnUrl = "")
    {
        var response = await _carService.GetCarByIdAsync(id);
     if (response.Successfull && response.Data != null)
   {
         _cart.AddToCart(response.Data);
        }

        return Redirect(string.IsNullOrEmpty(returnUrl) ? "/" : returnUrl);
    }

    /// <summary>
    /// Удалить автомобиль из корзины
    /// </summary>
    [Route("remove/{id:int}")]
    public IActionResult Remove(int id, string returnUrl = "")
    {
        _cart.RemoveItems(id);
      return Redirect(string.IsNullOrEmpty(returnUrl) ? "/" : returnUrl);
    }

    /// <summary>
    /// Очистить корзину
 /// </summary>
    [Route("clear")]
    public IActionResult Clear(string returnUrl = "")
    {
        _cart.ClearAll();
        return Redirect(string.IsNullOrEmpty(returnUrl) ? "/" : returnUrl);
    }

    /// <summary>
    /// Просмотр корзины
    /// </summary>
    [Route("")]
    [AllowAnonymous]
    public IActionResult Index()
    {
        return View(_cart);
    }
}