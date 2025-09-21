using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Rendering;
using Lab1.Models;
using System.Collections.Generic;

namespace Lab1.UI.Controllers
{
    public class HomeController : Controller
    {
        public IActionResult Index()
        {
            ViewData["LabWorkTitle"] = "Лабораторная работа №2";
            List<string> items1 = new List<string>
            {
                "элемент 1 списка",
                "элемент 2 списка",
                "элемент 3 списка"
            };
            ViewBag.Items = items1;
            var items = new List<ListDemo>
            {
                new ListDemo { Id = 1, Name = "Item 1" },
                new ListDemo { Id = 2, Name = "Item 2" },
                new ListDemo { Id = 3, Name = "Item 3" }
            };
            ViewBag.ListItems = new SelectList(items, "Id", "Name");
            
            return View();
        }
    }
}
