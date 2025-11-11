using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Options;
using Lab1;
using Lab1.Models;
using Lab1.Services.FileService;
using Microsoft.AspNetCore.Authorization;
using Lab1.Services.Authentication;

namespace Lab1.UI.Controllers;

[AllowAnonymous]
public class AccountController : Controller
{
    private readonly KeycloakData _kc;
    private readonly IFileService _fileService;

    public AccountController(IOptions<KeycloakData> kc, IFileService fileService)
    {
        _kc = kc.Value;
        _fileService = fileService;
    }

    [HttpGet("/login")]
    public IActionResult Login(string? returnUrl = null)
    {
        var props = new AuthenticationProperties
        {
            RedirectUri = string.IsNullOrEmpty(returnUrl)
                ? Url.Action("Index", "Home")!
                : returnUrl
        };
        return Challenge(props, "keycloak");
    }

    [Authorize]
    [HttpPost("/logout")]
    public async Task<IActionResult> Logout()
    {
        // Try to obtain id_token_hint from current user (must do before sign-out)
        var idToken = User?.FindFirst("id_token")?.Value;
        // Fallback: try to read token from auth properties (if SaveTokens was used)
        if (string.IsNullOrEmpty(idToken))
        {
            try { idToken = await HttpContext.GetTokenAsync("id_token"); } catch { /* ignore */ }
        }

        // Sign out local cookie
        await HttpContext.SignOutAsync("AppCookie");

        // Build Keycloak end session URL and redirect manually
        var host = (_kc?.Host ?? "http://localhost:8080").TrimEnd('/');
        var realm = _kc?.Realm ?? string.Empty;
        var postLogout = Url.Action("Index", "Home", null, Request.Scheme) ?? $"{Request.Scheme}://{Request.Host}/";

        var qs = new List<string> { "post_logout_redirect_uri=" + Uri.EscapeDataString(postLogout) };
        if (!string.IsNullOrEmpty(idToken))
            qs.Insert(0, "id_token_hint=" + Uri.EscapeDataString(idToken));

        var logoutUrl = $"{host}/realms/{realm}/protocol/openid-connect/logout?{string.Join('&', qs)}";
        Console.WriteLine("DEBUG logoutUrl: " + logoutUrl);

        return Redirect(logoutUrl);
    }

    [HttpGet("/Account/LoginFailed")]
    public IActionResult LoginFailed(string error) => Content("Login failed: " + error);

    [HttpGet("/Account/Register")]
    [AllowAnonymous]
    public IActionResult Register()
    {
        return View(new RegisterUserViewModel());
    }

    [HttpPost("/Account/Register")]
    [ValidateAntiForgeryToken]
    [AllowAnonymous]
    public async Task<IActionResult> Register(RegisterUserViewModel model, [FromServices] IKeycloakAdminService admin)
    {
        if (!ModelState.IsValid)
            return View(model);

        // Save avatar if provided
        string avatarUrl;
        if (model.Avatar is not null)
        {
            try
            {
                var url = await _fileService.SaveFileAsync(model.Avatar);
                Console.WriteLine("Saved avatar at " + url);
                avatarUrl = url;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error saving avatar: " + ex.Message);
                avatarUrl = "/images/default-avatar.png";
            }
        }
        else
        {
            avatarUrl = "/images/default-avatar.png";
        }

        var (success, error) = await admin.CreateUserAsync(model, avatarUrl);
        if (!success)
        {
            ModelState.AddModelError(string.Empty, "Failed to create user: " + error);
            return View(model);
        }

        return RedirectToAction("Login");
    }

    [HttpGet("/Account/AccessDenied")]
    [AllowAnonymous]
    public IActionResult AccessDenied(string? returnUrl = null)
    {
        ViewBag.ReturnUrl = returnUrl ?? Request.Query["ReturnUrl"].ToString();
        return View();
    }
}
