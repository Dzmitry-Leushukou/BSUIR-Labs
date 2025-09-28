using Lab1;
using Lab1.Extensions;
using Lab1.Services;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
var uriData = builder.Configuration.GetSection("UriData").Get<UriData>();
builder.Services.AddSingleton(uriData);

// Регистрация HttpClient для CarService
builder.Services.AddHttpClient<ICarService, ApiCarService>(opt =>
    opt.BaseAddress = new Uri(uriData.ApiUri + "cars/"));

// Регистрация HttpClient для CategoryService
builder.Services.AddHttpClient<ICategoryService, ApiCategoryService>(opt =>
    opt.BaseAddress = new Uri(uriData.ApiUri + "categories/"));
builder.Services.AddControllersWithViews();
builder.RegisterCustomServices();

var app = builder.Build();

// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Home/Error");
    // The default HSTS value is 30 days. You may want to change this for Carion scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();

app.UseRouting();

app.UseAuthorization();

app.MapControllerRoute(
    name: "default",
    pattern: "{controller=Home}/{action=Index}/{id?}");

app.Run();
