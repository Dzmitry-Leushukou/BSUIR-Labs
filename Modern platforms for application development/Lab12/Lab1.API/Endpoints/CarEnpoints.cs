using Lab1.API.Data;
using Lab1.API.UseCases;
using Lab1.Domain.Entities; // Car, Category
using Lab1.Domain.Models; // ResponseData<T>, ListModel<T>
using MediatR;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;
using System.Text.Json;
using Microsoft.Extensions.Caching.Hybrid;

namespace Lab1.API.Endpoints
{
    public static class CarEndpoints
    {
        private static readonly JsonSerializerOptions JsonOpts = new()
        {
            PropertyNameCaseInsensitive = true
        };

        public static void MapCarEndpoints(this IEndpointRouteBuilder routes)
        {
            // remove group-level admin requirement; apply per-endpoint
            var group = routes.MapGroup("/api/cars").DisableAntiforgery();

            // ---------- helpers ----------
            static string? ExtractImageFileName(string? imagePath)
            {
                if (string.IsNullOrWhiteSpace(imagePath)) return null;

                var normalized = imagePath.Replace("\\", "/");
                if (normalized.EndsWith("noimage.jpg", StringComparison.OrdinalIgnoreCase))
                    return null;

                if (Uri.TryCreate(normalized, UriKind.Absolute, out var uri))
                {
                    var path = uri.AbsolutePath; // "/Images/xxx.png"
                    var idx = path.LastIndexOf("/Images/", StringComparison.OrdinalIgnoreCase);
                    if (idx >= 0) return path[(idx + "/Images/".Length)..];
                    return Path.GetFileName(path);
                }

                normalized = normalized.TrimStart('/');
                if (normalized.StartsWith("Images/", StringComparison.OrdinalIgnoreCase))
                    return normalized["Images/".Length..];

                return Path.GetFileName(normalized);
            }

            static async Task<Category?> ResolveCategoryAsync(AppDbContext db, Category? incoming)
            {
                if (incoming is null) return null;

                if (incoming.Id > 0)
                {
                    var byId = await db.Categories.FindAsync(incoming.Id);
                    if (byId != null) return byId;
                }

                if (!string.IsNullOrWhiteSpace(incoming.NormalizedName))
                {
                    var byNorm = await db.Categories
                        .FirstOrDefaultAsync(x => x.NormalizedName == incoming.NormalizedName);
                    if (byNorm != null) return byNorm;
                }

                if (!string.IsNullOrWhiteSpace(incoming.Name))
                {
                    var byName = await db.Categories.FirstOrDefaultAsync(x => x.Name == incoming.Name);
                    if (byName != null) return byName;
                }

                return null;
            }

            static string EnsureName(string? name, IFormFile? file)
            {
                if (!string.IsNullOrWhiteSpace(name)) return name!;
                if (file != null)
                {
                    var n = Path.GetFileNameWithoutExtension(file.FileName);
                    if (!string.IsNullOrWhiteSpace(n)) return n;
                }
                // fallback, чтобы не падать на NOT NULL в SQLite
                return "Без названия";
            }

            // ---------- GET list (с категорией) ----------
            // GET /api/cars/{category?}?pageNo=1&pageSize=3
            group.MapGet("/{category:alpha?}", async (
                AppDbContext db,
                HybridCache cache,
                ILogger<Program> logger,
                string? category = null,
                int pageNo = 1,
                int pageSize = 3) =>
            {
                if (pageNo < 1) pageNo = 1;
                if (pageSize < 1) pageSize = 3;

                // Кэширование списка автомобилей
                var cacheKey = $"cars_{category}_{pageNo}_{pageSize}";
                logger.LogInformation("🔍 Cache key: {CacheKey}", cacheKey);

                var list = await cache.GetOrCreateAsync(
                    cacheKey,
                    async token =>
                    {
                        logger.LogInformation("💾 Cache MISS - fetching from DB for key: {CacheKey}", cacheKey);

                        IQueryable<Car> query = db.Cars
                            .AsNoTracking()
                            .Include(c => c.Category);

                        if (!string.IsNullOrWhiteSpace(category))
                            query = query.Where(p => p.Category != null && p.Category.NormalizedName == category);

                        var total = await query.CountAsync();
                        var items = await query
                            .OrderBy(p => p.Id)
                            .Skip((pageNo - 1) * pageSize)
                            .Take(pageSize)
                            .ToListAsync();

                        logger.LogInformation("✅ Loaded {Count} items from DB", items.Count);

                        return new ListModel<Car>
                        {
                            Items = items,
                            CurrentPage = pageNo,
                            TotalPages = (int)Math.Ceiling(total / (double)pageSize)
                        };
                    },
                    options: new HybridCacheEntryOptions
                    {
                        Expiration = TimeSpan.FromMinutes(1),
                        LocalCacheExpiration = TimeSpan.FromSeconds(30)
                    });

                logger.LogInformation("✨ Cache HIT or populated - returning {Count} items", list.Items.Count);
                return Results.Ok(new ResponseData<ListModel<Car>> { Data = list, Successfull = true });
            })
            .WithName("GetCars")
            .WithOpenApi()
            .RequireAuthorization(); ;

            // ---------- GET by id (с категией) ----------
            group.MapGet("/{id:int}", async (int id, AppDbContext db) =>
            {
                var car = await db.Cars
                    .AsNoTracking()
                    .Include(c => c.Category)
                    .FirstOrDefaultAsync(x => x.Id == id);

                return car is null
                    ? Results.NotFound(ResponseData<Car>.Error("Not found"))
                    : Results.Ok(new ResponseData<Car> { Data = car, Successfull = true });
            })
            .WithName("GetCarById")
            .WithOpenApi()
            .AllowAnonymous(); ;

            // ---------- POST: multipart (car + file) ----------
            group.MapPost("/", async (
                [FromForm] string car,
                [FromForm] IFormFile? file,
                AppDbContext db,
                IMediator mediator,
                HttpRequest req,
                ILogger<Program> logger,
                ClaimsPrincipal user) =>
            {
                // log Authorization header for debugging
                if (req.Headers.TryGetValue("Authorization", out var ah))
                {
                    logger.LogDebug("Incoming Authorization header length: {Len}", ah.ToString().Length);
                }
                else
                {
                    logger.LogDebug("No Authorization header present on request");
                }

                // Log user identity info
                try
                {
                    logger.LogDebug("User.Identity.IsAuthenticated={IsAuth}", user?.Identity?.IsAuthenticated == true);
                    if (user?.Identity?.IsAuthenticated == true)
                    {
                        var name = user.Identity?.Name ?? user.FindFirst("preferred_username")?.Value ?? "(no name)";
                        logger.LogDebug("User name: {Name}", name);
                        var roles = user.Claims.Where(c => c.Type == ClaimTypes.Role || c.Type == "role").Select(c => c.Value).ToList();
                        logger.LogDebug("User roles: {Roles}", roles.Count >0 ? string.Join(',', roles) : "(none)");
                        logger.LogDebug("All claims count: {Count}", user.Claims.Count());
                    }
                }
                catch (Exception ex)
                {
                    logger.LogError(ex, "Error logging user claims");
                }

                var entity = JsonSerializer.Deserialize<Car>(car, JsonOpts);
                if (entity is null)
                    return Results.BadRequest(ResponseData<Car>.Error("Bad payload: car JSON"));

                // безопасно заполняем обязательные поля
                entity.Name = EnsureName(entity.Name, file);
                entity.Category = await ResolveCategoryAsync(db, entity.Category);

                if (file != null)
                {
                    entity.ImagePath = await mediator.Send(new SaveImage(file));
                    entity.Mime = file.ContentType;
                }
                else
                {
                    entity.ImagePath ??= "Images/noimage.jpg";
                    entity.Mime ??= "image/jpeg";
                }

                db.Cars.Add(entity);
                await db.SaveChangesAsync();

                // для ответа подгружаем навигацию
                await db.Entry(entity).Reference(e => e.Category).LoadAsync();

                return Results.Created($"/api/cars/{entity.Id}",
                    new ResponseData<Car> { Data = entity, Successfull = true });
            })
            .WithName("CreateCar")
            .WithOpenApi()
            .RequireAuthorization();

            // ---------- PUT: multipart (car + optional file) ----------
            group.MapPut("/{id:int}", async (
                int id,
                [FromForm] string car,
                [FromForm] IFormFile? file,
                AppDbContext db,
                IMediator mediator,
                IWebHostEnvironment env) =>
            {
                var existing = await db.Cars.FirstOrDefaultAsync(x => x.Id == id);
                if (existing is null) return Results.NotFound(ResponseData<Car>.Error("Not found"));

                var incoming = JsonSerializer.Deserialize<Car>(car, JsonOpts);
                if (incoming is null)
                    return Results.BadRequest(ResponseData<Car>.Error("Bad payload: car JSON"));

                existing.Name = EnsureName(incoming.Name, file);
                existing.Description = incoming.Description;
                existing.Price = incoming.Price;

                // Категория — по навигации
                existing.Category = await ResolveCategoryAsync(db, incoming.Category);

                if (file != null)
                {
                    var oldName = ExtractImageFileName(existing.ImagePath);
                    if (!string.IsNullOrEmpty(oldName))
                    {
                        var root = env.WebRootPath ?? Path.Combine(Directory.GetCurrentDirectory(), "wwwroot");
                        var fullOld = Path.Combine(root, "Images", oldName);
                        if (System.IO.File.Exists(fullOld))
                        {
                            try { System.IO.File.Delete(fullOld); } catch { /* ignore */ }
                        }
                    }

                    existing.ImagePath = await mediator.Send(new SaveImage(file));
                    existing.Mime = file.ContentType;
                }

                await db.SaveChangesAsync();
                return Results.NoContent();
            })
            .WithName("UpdateCar")
            .WithOpenApi()
            .RequireAuthorization("admin");

            // ---------- DELETE: с удалением файла ----------
            group.MapDelete("/{id:int}", async (int id, AppDbContext db, IWebHostEnvironment env) =>
            {
                var entity = await db.Cars.FirstOrDefaultAsync(x => x.Id == id);
                if (entity is null) return Results.NotFound(ResponseData<Car>.Error("Not found"));

                var fileName = ExtractImageFileName(entity.ImagePath);
                if (!string.IsNullOrEmpty(fileName))
                {
                    var root = env.WebRootPath ?? Path.Combine(Directory.GetCurrentDirectory(), "wwwroot");
                    var full = Path.Combine(root, "Images", fileName);
                    if (System.IO.File.Exists(full))
                    {
                        try { System.IO.File.Delete(full); } catch { /* ignore */ }
                    }
                }

                db.Cars.Remove(entity);
                await db.SaveChangesAsync();
                return Results.Ok(new ResponseData<Car> { Data = entity, Successfull = true });
            })
            .WithName("DeleteCar")
            .WithOpenApi()
            .RequireAuthorization("admin");
        }
    }
}
