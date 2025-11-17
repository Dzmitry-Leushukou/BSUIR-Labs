using Lab1.API.Data;
using Lab1.API.Endpoints;
using Lab1.API.UseCases;
using MediatR;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.EntityFrameworkCore;
using Lab1.API.Models;
using Microsoft.IdentityModel.Tokens;
using System.Text.Json;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using Microsoft.Extensions.Caching.Hybrid;
using Microsoft.Extensions.Caching.Distributed;
using StackExchange.Redis;
using Microsoft.Extensions.Caching.StackExchangeRedis;

var builder = WebApplication.CreateBuilder(args);

// ---- DB (SQLite) ----
var cs = builder.Configuration.GetConnectionString("SQLite") ?? "Data Source=menu.db";
builder.Services.AddDbContext<AppDbContext>(opt => opt.UseSqlite(cs));

// ---- MVC + Swagger ----
builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// ---- MediatR v13 ----
builder.Services.AddMediatR(cfg => cfg.RegisterServicesFromAssemblyContaining<GetListOfCars>());

// ---- Caching (Hybrid Cache + Redis) ----
builder.Services.AddHybridCache();

var redisConnectionString = builder.Configuration.GetConnectionString("Redis");
if (!string.IsNullOrEmpty(redisConnectionString))
{
    try
    {
        // Парсим строку подключения Redis
      var redisConfig = ConfigurationOptions.Parse(redisConnectionString);
 redisConfig.AbortOnConnectFail = false;  // Не падать при ошибке подключения
        redisConfig.ConnectTimeout = 3000;       // 3 сек таймаут
        redisConfig.SyncTimeout = 3000;
        redisConfig.AllowAdmin = false;

 builder.Services.AddStackExchangeRedisCache(opt =>
   {
  opt.InstanceName = "cars_";
            opt.Configuration = redisConnectionString;
    opt.ConfigurationOptions = redisConfig;
        });
  Console.WriteLine("✅ Redis подключен на " + redisConnectionString);
    }
    catch (Exception ex)
    {
        // Redis недоступен - используем только локальный кэш
        Console.WriteLine("⚠️  Redis недоступен: " + ex.Message + ". Используется только локальный кэш.");
    }
}
else
{
    Console.WriteLine("ℹ️  Строка подключения Redis не найдена. Используется только локальный кэш.");
}

// ---- HttpContext (нужен для SaveImageHandler) ----
builder.Services.AddHttpContextAccessor();

// ---- AuthN/AuthZ (Keycloak) ----
var authServer = builder.Configuration.GetSection("AuthServer").Get<AuthServerData>() ?? new AuthServerData();
builder.Services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
 .AddJwtBearer(JwtBearerDefaults.AuthenticationScheme, o =>
 {
 o.MetadataAddress = $"{authServer.Host}/realms/{authServer.Realm}/.well-known/openid-configuration";
 o.Authority = $"{authServer.Host}/realms/{authServer.Realm}";
 o.RequireHttpsMetadata = false; // локально

 // Do not require a specific audience for local development; validate issuer and roles instead
 o.TokenValidationParameters = new TokenValidationParameters
 {
 ValidateAudience = false,
 ValidateIssuer = true,
 // Accept both localhost and127.0.0.1 issuer variants (local dev)
 ValidIssuers = new[] {
 (authServer.Host ?? "http://localhost:8080").TrimEnd('/') + $"/realms/{authServer.Realm}",
 (authServer.Host ?? "http://localhost:8080").Replace("127.0.0.1", "localhost").TrimEnd('/') + $"/realms/{authServer.Realm}",
 (authServer.Host ?? "http://localhost:8080").Replace("localhost", "127.0.0.1").TrimEnd('/') + $"/realms/{authServer.Realm}"
 },
 NameClaimType = "preferred_username",
 RoleClaimType = "role"
 };

 // Extract roles from nested token claims (realm_access and resource_access) and add as top-level role claims
 o.Events = new JwtBearerEvents
 {
 OnMessageReceived = ctx =>
 {
 try
 {
 var req = ctx.HttpContext.Request;
 if (req.Headers.TryGetValue("Authorization", out var ah))
 {
 var token = ah.ToString();
 // log length only
 ctx.HttpContext.RequestServices.GetRequiredService<ILoggerFactory>()
 .CreateLogger("JwtBearer").LogDebug("Incoming Authorization header length: {Len}", token.Length);
 }
 else
 {
 ctx.HttpContext.RequestServices.GetRequiredService<ILoggerFactory>()
 .CreateLogger("JwtBearer").LogDebug("No Authorization header present on request");
 }
 }
 catch { }
 return Task.CompletedTask;
 },
 OnTokenValidated = ctx =>
 {
 try
 {
 var jwt = ctx.SecurityToken as JwtSecurityToken;
 if (jwt != null)
 {
 void AddRolesFromObject(object? obj)
 {
 if (obj == null) return;
 JsonElement je;
 if (obj is JsonElement jex)
 {
 je = jex;
 }
 else
 {
 je = JsonSerializer.Deserialize<JsonElement>(JsonSerializer.Serialize(obj));
 }
 if (je.ValueKind == JsonValueKind.Object && je.TryGetProperty("roles", out var rolesJe) && rolesJe.ValueKind == JsonValueKind.Array)
 {
 foreach (var r in rolesJe.EnumerateArray())
 {
 var role = r.GetString();
 if (!string.IsNullOrEmpty(role))
 {
 var ci = ctx.Principal.Identity as ClaimsIdentity;
 ci?.AddClaim(new Claim("role", role));
 }
 }
 }
 else if (je.ValueKind == JsonValueKind.Object)
 {
 // Possibly a map of client->{roles: []}
 foreach (var prop in je.EnumerateObject())
 {
 if (prop.Value.ValueKind == JsonValueKind.Object && prop.Value.TryGetProperty("roles", out var rarr) && rarr.ValueKind == JsonValueKind.Array)
 {
 foreach (var r in rarr.EnumerateArray())
 {
 var role = r.GetString();
 if (!string.IsNullOrEmpty(role))
 {
 var ci = ctx.Principal.Identity as ClaimsIdentity;
 ci?.AddClaim(new Claim("role", role));
 }
 }
 }
 }
 }
 }

 if (jwt.Payload.TryGetValue("realm_access", out var realmAccess))
 {
 AddRolesFromObject(realmAccess);
 }
 if (jwt.Payload.TryGetValue("resource_access", out var resourceAccess))
 {
 AddRolesFromObject(resourceAccess);
 }
 }
 }
 catch (Exception ex)
 {
 ctx.HttpContext.RequestServices.GetRequiredService<ILoggerFactory>()
 .CreateLogger("JwtBearer").LogError(ex, "Error while extracting roles from token");
 }
 return Task.CompletedTask;
 },
 OnAuthenticationFailed = ctx =>
 {
 ctx.HttpContext.RequestServices.GetRequiredService<ILoggerFactory>()
 .CreateLogger("JwtBearer").LogError(ctx.Exception, "JWT authentication failed");
 return Task.CompletedTask;
 }
 };
 });

builder.Services.AddAuthorization(opt =>
{
 opt.AddPolicy("admin", p => p.RequireRole("POWER-USER"));
});

var app = builder.Build();

app.UseStaticFiles();

if (app.Environment.IsDevelopment())
{
 app.UseSwagger();
 app.UseSwaggerUI();
}

app.UseAuthentication();
app.UseAuthorization();

// ===== Middleware для мониторинга Redis =====
app.Use(async (context, next) =>
{
    var distributedCache = context.RequestServices.GetService<IDistributedCache>();
    if (distributedCache != null && context.Request.Path.StartsWithSegments("/api/cars"))
    {
        var logger = context.RequestServices.GetRequiredService<ILogger<Program>>();
      
    var cacheType = distributedCache.GetType().Name;
        logger.LogInformation("📦 Cache type in use: {CacheType}", cacheType);
    }
    
    await next();
});

app.MapControllers();
app.MapCarEndpoints();

app.Run();
