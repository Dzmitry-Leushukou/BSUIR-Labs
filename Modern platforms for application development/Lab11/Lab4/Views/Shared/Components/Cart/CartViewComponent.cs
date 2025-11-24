using Microsoft.AspNetCore.Mvc;
using Lab1.Domain.Models;

public class CartViewComponent : ViewComponent
{
    private readonly Cart _cart;

    public CartViewComponent(Cart cart)
    {
      _cart = cart;
    }

    public IViewComponentResult Invoke()
    {
        return View(_cart);
    }
}
