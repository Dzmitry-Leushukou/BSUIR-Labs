using Lab1.Domain.Entities;
using Microsoft.EntityFrameworkCore;

namespace Lab1.API.Data;

public class AppDbContext : DbContext
{
    public AppDbContext(DbContextOptions<AppDbContext> options) : base(options)
    {
    }
    public DbSet<Car> Cars { get; set; }
    public DbSet<Category> Categories { get; set; }
}