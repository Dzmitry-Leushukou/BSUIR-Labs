using Lab1;
using Lab1.Extensions;
using Lab1.Services;                  // ← ОБЯЗАТЕЛЬНО: тут лежит ApiCarService / ApiCategoryService
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;

var builder = WebApplication.CreateBuilder(args);

// UriData из appsettings.json
var uriData = builder.Configuration.GetSection("UriData").Get<UriData>() ?? new UriData();
builder.Services.AddSingleton(uriData);

// RazorPages + MVC
builder.Services.AddRazorPages();
builder.Services.AddControllersWithViews();

// Typed HttpClient
builder.Services.AddHttpClient<ICarService, ApiCarService>(client =>
{
    client.BaseAddress = new Uri($"{uriData.ApiUri}cars/");
});

builder.Services.AddHttpClient<ICategoryService, ApiCategoryService>(client =>
{
    client.BaseAddress = new Uri($"{uriData.ApiUri}categories/");
});

var app = builder.Build();

if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Home/Error");
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();

app.UseRouting();
app.UseAuthorization();

app.MapRazorPages();

app.MapControllerRoute(
    name: "default",
    pattern: "{controller=Home}/{action=Index}/{id?}");

app.Run();
