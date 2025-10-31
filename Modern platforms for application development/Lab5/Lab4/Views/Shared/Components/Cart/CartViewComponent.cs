using Microsoft.AspNetCore.Mvc;

public class CartViewComponent : ViewComponent
{
    public IViewComponentResult Invoke()
    {
        return View();
    }
}
