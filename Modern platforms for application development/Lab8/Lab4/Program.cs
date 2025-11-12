using Lab1.Extensions;
using Lab1.Services;
using Lab1.Services.CarService;
using Lab1.Services.CategoryService;
using Lab1.Services.Authentication;
using Lab1.Services.FileService;

using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Authentication.OpenIdConnect;
using Microsoft.AspNetCore.Authentication;
using System.Text.Json;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.IdentityModel.Logging;
using Microsoft.IdentityModel.Tokens;
using Microsoft.IdentityModel.Protocols;
using Microsoft.IdentityModel.Protocols.OpenIdConnect;
using System.Net.Http.Headers;
using System.Security.Claims;

using Lab1; // add types KeycloakData, UriData

var builder = WebApplication.CreateBuilder(args);

// Register fallback in-memory services and configuration used by extensions
builder.RegisterCustomServices();

// ===== конфиги =====
builder.Services.Configure<UriData>(builder.Configuration.GetSection("UriData"));
builder.Services.Configure<KeycloakData>(builder.Configuration.GetSection("Keycloak"));
var uriData = builder.Configuration.GetSection("UriData").Get<UriData>() ?? new UriData();
var kc = builder.Configuration.GetSection("Keycloak").Get<KeycloakData>() ?? new KeycloakData();

// ===== HTTP-клиенты =====
builder.Services.AddHttpClient<ICarService, ApiCarService>(c => c.BaseAddress = new Uri($"{uriData.ApiUri}cars/"));
builder.Services.AddHttpClient<ICategoryService, ApiCategoryService>(c => c.BaseAddress = new Uri($"{uriData.ApiUri}categories/"));

builder.Services.AddHttpContextAccessor();
builder.Services.AddHttpClient<ITokenAccessor, KeycloakTokenAccessor>();
// Keycloak admin service for user creation
builder.Services.AddHttpClient<IKeycloakAdminService, KeycloakAdminService>();
builder.Services.AddScoped<IFileService, LocalFileService>();

builder.Services.AddControllersWithViews();
builder.Services.AddRazorPages();

// Ensure images folder exists
var imagesDir = Path.Combine(builder.Environment.ContentRootPath ?? ".", "wwwroot", "images");
if (!Directory.Exists(imagesDir)) Directory.CreateDirectory(imagesDir);

// подробные логи
IdentityModelEventSource.ShowPII = true;

// ============= AUTH =============
builder.Services
 .AddAuthentication(o =>
 {
 o.DefaultScheme = "AppCookie";
 o.DefaultChallengeScheme = "keycloak"; // ВАЖНО: совпадает с вызовом Challenge в контроллере
 })
 .AddCookie("AppCookie", o =>
 {
 o.Cookie.Name = "lab4.app";
 o.Cookie.SameSite = SameSiteMode.None;
 o.Cookie.SecurePolicy = CookieSecurePolicy.Always;
 o.SlidingExpiration = true;
 // Redirect here when an authenticated user is forbidden (403)
 o.AccessDeniedPath = "/Account/AccessDenied";
 })
 .AddOpenIdConnect("keycloak", options =>
 {
 // попробуем разные хосты (берём из конфигура, затем127.0.0.1, затем localhost)
 var hosts = new[]
 {
 kc.Host?.TrimEnd('/'),
 "http://127.0.0.1:8080",
 "http://localhost:8080"
 }.Where(h => !string.IsNullOrWhiteSpace(h)).Distinct().ToArray();

 OpenIdConnectConfiguration? conf = null;
 string? realmBaseUsed = null;

 foreach (var host in hosts)
 {
 try
 {
 var realmBase = $"{host}/realms/{kc.Realm}";
 var metadata = $"{realmBase}/.well-known/openid-configuration";

 var retriever = new HttpDocumentRetriever { RequireHttps = false };

 // discovery
 var mdJson = retriever.GetDocumentAsync(metadata, CancellationToken.None).GetAwaiter().GetResult();
 var loaded = OpenIdConnectConfiguration.Create(mdJson);

 // jwks
 var jwksJson = retriever.GetDocumentAsync(loaded.JwksUri, CancellationToken.None).GetAwaiter().GetResult();
 foreach (var k in new JsonWebKeySet(jwksJson).GetSigningKeys())
 loaded.SigningKeys.Add(k);

 conf = loaded;
 realmBaseUsed = realmBase;
 break;
 }
 catch
 {
 // пробуем следующий хост
 }
 }

 if (conf is null || realmBaseUsed is null)
 {
 var rb = $"{(kc.Host ?? "http://localhost:8080").TrimEnd('/')}/realms/{kc.Realm}";
 conf = new OpenIdConnectConfiguration
 {
 AuthorizationEndpoint = $"{rb}/protocol/openid-connect/auth",
 TokenEndpoint = $"{rb}/protocol/openid-connect/token",
 UserInfoEndpoint = $"{rb}/protocol/openid-connect/userinfo",
 EndSessionEndpoint = $"{rb}/protocol/openid-connect/logout",
 JwksUri = $"{rb}/protocol/openid-connect/certs",
 Issuer = $"{rb}"
 };
 realmBaseUsed = rb;
 try
 {
 var retriever = new HttpDocumentRetriever { RequireHttps = false };
 var jwksJson = retriever.GetDocumentAsync(conf.JwksUri, CancellationToken.None).GetAwaiter().GetResult();
 foreach (var k in new JsonWebKeySet(jwksJson).GetSigningKeys())
 conf.SigningKeys.Add(k);
 }
 catch { }
 }

 options.RequireHttpsMetadata = false; // Keycloak по http
 options.Authority = realmBaseUsed;
 options.MetadataAddress = $"{realmBaseUsed}/.well-known/openid-configuration";
 options.Configuration = conf;

 options.ClientId = kc.ClientId;
 options.ClientSecret = kc.ClientSecret;
 // Use pure authorization code flow and perform manual token exchange to avoid middleware id_token validation
 options.ResponseType = OpenIdConnectResponseType.Code;
 // Disable PKCE to simplify exchange (we will send client_secret)
 options.UsePkce = false;

 options.Scope.Clear();
 options.Scope.Add("openid");
 options.Scope.Add("profile");
 options.Scope.Add("email");

 options.SaveTokens = true;
 options.GetClaimsFromUserInfoEndpoint = true;
 options.MapInboundClaims = false;

 options.CallbackPath = "/signin-oidc";
 options.SignedOutCallbackPath = "/signout-callback-oidc";

 options.CorrelationCookie.SameSite = SameSiteMode.None;
 options.CorrelationCookie.SecurePolicy = CookieSecurePolicy.Always;
 options.NonceCookie.SameSite = SameSiteMode.None;
 options.NonceCookie.SecurePolicy = CookieSecurePolicy.Always;

 options.TokenValidationParameters = new TokenValidationParameters
 {
 ValidateIssuer = true,
 ValidIssuers = new[] {
 $"http://127.0.0.1:8080/realms/{kc.Realm}",
 $"http://localhost:8080/realms/{kc.Realm}"
 },
 ValidateAudience = false,
 NameClaimType = "preferred_username",
 RoleClaimType = "role",
 IssuerSigningKeys = conf.SigningKeys // если подгрузили
 };

 options.Events = new OpenIdConnectEvents
 {
 OnRedirectToIdentityProvider = ctx =>
 {
 var baseUrl = realmBaseUsed;
 ctx.ProtocolMessage.IssuerAddress = $"{baseUrl}/protocol/openid-connect/auth";
 return Task.CompletedTask;
 },
 // Prevent middleware from trying to validate id_token (which may lack 'aud') by removing it early
 OnMessageReceived = ctx =>
 {
 try
 {
 // Remove id_token so the protocol validator does not attempt to validate it
 if (!string.IsNullOrEmpty(ctx.ProtocolMessage?.IdToken))
 {
 Console.WriteLine("Clearing id_token from protocol message to avoid validator 'aud' check.");
 ctx.ProtocolMessage.IdToken = null;
 }
 }
 catch { }
 return Task.CompletedTask;
 },
 // Manual code exchange: exchange the authorization code for tokens and create local cookie identity using UserInfo
 OnAuthorizationCodeReceived = async ctx =>
 {
 try
 {
 var tokenEndpoint = ctx.Options.Configuration.TokenEndpoint;
 var clientFactory = ctx.HttpContext.RequestServices.GetRequiredService<IHttpClientFactory>();
 var client = clientFactory.CreateClient();

 // Prefer stored redirect used by the handler
 var redirectUri = ctx.Properties?.Items != null && ctx.Properties.Items.TryGetValue("OpenIdConnect.Code.RedirectUri", out var stored) && !string.IsNullOrEmpty(stored)
 ? stored
 : (ctx.Properties.RedirectUri ?? (ctx.Request.Scheme + "://" + ctx.Request.Host + ctx.Request.PathBase + ctx.Options.CallbackPath));

 var form = new List<KeyValuePair<string,string>>
 {
 new("grant_type","authorization_code"),
 new("code", ctx.ProtocolMessage.Code),
 new("redirect_uri", redirectUri),
 new("client_id", ctx.Options.ClientId),
 new("client_secret", ctx.Options.ClientSecret)
 };

 var req = new HttpRequestMessage(HttpMethod.Post, tokenEndpoint)
 {
 Content = new FormUrlEncodedContent(form)
 };
 var resp = await client.SendAsync(req);
 resp.EnsureSuccessStatusCode();
 var payload = JsonDocument.Parse(await resp.Content.ReadAsStringAsync()).RootElement;
 var accessToken = payload.TryGetProperty("access_token", out var at) ? at.GetString() : null;
 var idToken = payload.TryGetProperty("id_token", out var it) ? it.GetString() : null;
 var refreshToken = payload.TryGetProperty("refresh_token", out var rt) ? rt.GetString() : null;
 // If id_token exists, ignore it to avoid aud validation issues; rely on UserInfo

 if (!string.IsNullOrEmpty(accessToken) && !string.IsNullOrEmpty(ctx.Options.Configuration.UserInfoEndpoint))
 {
 var uiReq = new HttpRequestMessage(HttpMethod.Get, ctx.Options.Configuration.UserInfoEndpoint);
 uiReq.Headers.Authorization = new AuthenticationHeaderValue("Bearer", accessToken);
 var uiResp = await client.SendAsync(uiReq);
 uiResp.EnsureSuccessStatusCode();
 var uiJson = JsonDocument.Parse(await uiResp.Content.ReadAsStringAsync()).RootElement;

 var claims = new List<Claim>();
 foreach (var prop in uiJson.EnumerateObject())
 {
 if (prop.Value.ValueKind == JsonValueKind.Array)
 {
 foreach (var v in prop.Value.EnumerateArray())
 {
 claims.Add(new Claim(prop.Name, v.ToString()));
 }
 }
 else
 {
 claims.Add(new Claim(prop.Name, prop.Value.ToString()));
 }
 }

 // Ensure name/role claims mapping
 var identity = new ClaimsIdentity(claims, "AppCookie", "preferred_username", "role");
 var principal = new ClaimsPrincipal(identity);

 var authProps = new AuthenticationProperties();
 // Store tokens including id_token so logout can use id_token_hint
 var tokens = new List<AuthenticationToken>();
 if (!string.IsNullOrEmpty(accessToken)) tokens.Add(new AuthenticationToken { Name = "access_token", Value = accessToken });
 if (!string.IsNullOrEmpty(idToken)) tokens.Add(new AuthenticationToken { Name = "id_token", Value = idToken });
 if (!string.IsNullOrEmpty(refreshToken)) tokens.Add(new AuthenticationToken { Name = "refresh_token", Value = refreshToken });
 if (tokens.Count >0) authProps.StoreTokens(tokens);

 await ctx.HttpContext.SignInAsync("AppCookie", principal, authProps);
 // Redirect back to original return URL (stored in auth properties as .redirect) or default to '/'
 string returnUrl = "/";
 try
 {
 if (ctx.Properties?.Items != null && ctx.Properties.Items.TryGetValue(".redirect", out var storedRedirect) && !string.IsNullOrEmpty(storedRedirect))
 {
 var sr = storedRedirect;
 if (sr.StartsWith("/"))
 {
 returnUrl = ctx.Request.Scheme + "://" + ctx.Request.Host + sr;
 }
 else
 {
 returnUrl = sr;
 }
 }
 else if (!string.IsNullOrEmpty(ctx.Properties?.RedirectUri))
 {
 returnUrl = ctx.Properties.RedirectUri;
 }
 }
 catch { }
 Console.WriteLine("Redirecting to after signin: " + returnUrl);
 ctx.Response.Redirect(returnUrl);
 // Tell the OIDC handler we handled the response
 ctx.HandleResponse();
 }
 }
 catch (Exception ex)
 {
 Console.WriteLine("OnAuthorizationCodeReceived error: " + ex);
 ctx.HandleResponse();
 }
 },
 OnRemoteFailure = ctx =>
 {
 ctx.HandleResponse();
 var msg = Uri.EscapeDataString(ctx.Failure?.Message ?? "unknown");
 ctx.Response.Redirect("/Account/LoginFailed?error=" + msg);
 return Task.CompletedTask;
 }
 };
 });

builder.Services.AddAuthorization(opt =>
{
 opt.AddPolicy("admin", p => p.RequireRole("POWER-USER"));
});

// ============= PIPELINE =============
var app = builder.Build();

app.UseDeveloperExceptionPage();

app.UseHttpsRedirection();
app.UseStaticFiles();
app.UseRouting();

app.UseAuthentication();
app.UseAuthorization();

// Handle OIDC post-logout callback so Keycloak redirect has a target
app.MapGet("/signout-callback-oidc", async ctx =>
{
 try { await ctx.SignOutAsync("AppCookie"); } catch { }
 var home = $"{ctx.Request.Scheme}://{ctx.Request.Host}/";
 Console.WriteLine("DEBUG: signout-callback-oidc hit, redirecting to " + home);
 ctx.Response.Redirect(home);
 await Task.CompletedTask;
});

app.MapRazorPages().RequireAuthorization("admin");
app.MapControllerRoute(name: "default", pattern: "{controller=Home}/{action=Index}/{id?}");

app.Run();
