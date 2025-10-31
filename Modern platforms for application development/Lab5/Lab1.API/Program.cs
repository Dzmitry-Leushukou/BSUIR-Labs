using Lab1.API.Data;
using Lab1.API.Endpoints;
using Lab1.API.UseCases;
using MediatR;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

var cs = builder.Configuration.GetConnectionString("SQLite") ?? "Data Source=menu.db";
builder.Services.AddDbContext<AppDbContext>(opt => opt.UseSqlite(cs));

builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

builder.Services.AddHttpContextAccessor();
builder.Services.Configure<FormOptions>(o => { o.MultipartBodyLengthLimit = 10 * 1024 * 1024; });

builder.Services.AddMediatR(cfg => cfg.RegisterServicesFromAssemblyContaining<SaveImageHandler>());

var app = builder.Build();

app.UseStaticFiles();

if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

// app.UseHttpsRedirection(); // ← можешь отключить для локалки, чтобы убрать warning

app.MapControllers();
app.MapCarEndpoints();

app.Run();
