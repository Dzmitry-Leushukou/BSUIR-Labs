using Lab1.API.Data;
using Lab1.Domain.Entities;
using Microsoft.EntityFrameworkCore;

namespace Lab1.API.Endpoints;

public static class CategoriesEndpoints
{
    public static void MapCategoriesEndpoints(this IEndpointRouteBuilder routes)
    {
        var group = routes.MapGroup("/api/categories").DisableAntiforgery();

        // GET: /api/categories
        group.MapGet("/", GetCategories)
            .WithName("GetAllCategories")
            .WithOpenApi()
            .AllowAnonymous();

        // GET: /api/categories/{id}
        group.MapGet("/{id}", GetCategory)
     .WithName("GetCategoryById")
 .WithOpenApi()
            .AllowAnonymous();
    }

    private static async Task<IResult> GetCategories(AppDbContext context)
    {
        try
        {
    var categories = await context.Categories.ToListAsync();
      return Results.Ok(categories);
        }
        catch
{
            return Results.StatusCode(500);
        }
    }

  private static async Task<IResult> GetCategory(int id, AppDbContext context)
    {
   try
        {
      var category = await context.Categories.FindAsync(id);
      if (category == null)
            return Results.NotFound();

            return Results.Ok(category);
        }
        catch
        {
         return Results.StatusCode(500);
   }
    }
}
