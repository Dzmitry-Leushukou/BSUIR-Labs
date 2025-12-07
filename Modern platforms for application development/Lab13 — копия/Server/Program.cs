using Server.Services;
using Server.Hubs;
using Microsoft.AspNetCore.Http.Connections;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.

// Add SignalR with proper WebSocket configuration
builder.Services.AddSignalR(options =>
{
    options.MaximumReceiveMessageSize = 1024 * 1024; // 1MB
    options.EnableDetailedErrors = true;
});

// Add game services
builder.Services.AddSingleton<GameSessionManager>();
builder.Services.AddSingleton<BattleshipGameLogic>();
builder.Services.AddSingleton<ConnectionGameMapper>();

builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Add CORS for Blazor WebAssembly
builder.Services.AddCors(options =>
{
    options.AddPolicy("BlazorCors", policy =>
    {
    if (builder.Environment.IsDevelopment())
        {
         policy
  .WithOrigins("https://localhost:7240", "https://localhost:7100", "https://localhost:5001", "http://localhost:3000", "http://localhost:5000")
         .AllowAnyMethod()
            .AllowAnyHeader()
            .AllowCredentials();
        }
        else
        {
            policy
      .WithOrigins("https://localhost:7240")
 .AllowAnyMethod()
        .AllowAnyHeader()
       .AllowCredentials();
        }
    });
});

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();

// Use CORS
app.UseCors("BlazorCors");

app.UseAuthorization();

app.MapControllers();

// Map SignalR hub with CORS options
app.MapHub<BattleshipHub>("/battleshipHub", options =>
{
    options.Transports = Microsoft.AspNetCore.Http.Connections.HttpTransportType.WebSockets |
       Microsoft.AspNetCore.Http.Connections.HttpTransportType.LongPolling;
});

app.Run();
