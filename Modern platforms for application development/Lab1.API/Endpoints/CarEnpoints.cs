using Lab1.API.Data;
using Lab1.API.UseCases;
using Lab1.Domain.Entities;
using MediatR;
using Microsoft.EntityFrameworkCore;

namespace Lab1.API.Endpoints;

public static class CarEndpoints
{
    public static void MapCarEndpoints(this IEndpointRouteBuilder routes)
    {
        var group = routes.MapGroup("/api/cars");

        group.MapGet("/{category:alpha?}", async (IMediator mediator, string? category, int pageNo = 1, int pageSize = 3) =>
        {
            var data = await mediator.Send(new GetListOfCars(category, pageNo, pageSize));
            return TypedResults.Ok(data);
        })
        .WithName("GetAllCars")
        .WithOpenApi();

        group.MapGet("/{id}", async (int id, AppDbContext db) =>
        {
            return await db.Cars.FindAsync(id)
                is Car model
                    ? Results.Ok(model)
                    : Results.NotFound();
        })
        .WithName("GetCarById")
        .WithOpenApi();

        group.MapPut("/{id}", async (int id, Car Car, AppDbContext db) =>
        {
            var foundModel = await db.Cars.FindAsync(id);

            if (foundModel is null)
            {
                return Results.NotFound();
            }

            // Обновляем свойства
            foundModel.Id= Car.Id;
            foundModel.Name = Car.Name;
            foundModel.Description = Car.Description;
            foundModel.Category = Car.Category;
            foundModel.Price = Car.Price;
            foundModel.ImagePath = Car.ImagePath;
            foundModel.Mime = Car.Mime;
    await db.SaveChangesAsync();

            return Results.NoContent();
        })
        .WithName("UpdateCar")
        .WithOpenApi();

        group.MapPost("/", async (Car Car, AppDbContext db) =>
        {
            db.Cars.Add(Car);
            await db.SaveChangesAsync();
            return Results.Created($"/api/Car/{Car.Id}", Car);
        })
        .WithName("CreateCar")
        .WithOpenApi();

        group.MapDelete("/{id}", async (int id, AppDbContext db) =>
        {
            if (await db.Cars.FindAsync(id) is Car Car)
            {
                db.Cars.Remove(Car);
                await db.SaveChangesAsync();
                return Results.Ok(Car);
            }

            return Results.NotFound();
        })
        .WithName("DeleteCar")
        .WithOpenApi();
    }
}