using System.Net.Http.Headers;
using System.Text.Json;
using System.Text;
using Lab1.Models;
using Microsoft.Extensions.Options;
using Lab1.Services.Authentication;

namespace Lab1.Services.Authentication
{
 public class KeycloakAdminService : IKeycloakAdminService
 {
 private readonly HttpClient _http;
 private readonly KeycloakData _kc;
 private readonly ITokenAccessor _tokenAccessor;

 public KeycloakAdminService(HttpClient http, IOptions<KeycloakData> options, ITokenAccessor tokenAccessor)
 {
 _http = http;
 _kc = options.Value;
 _tokenAccessor = tokenAccessor;
 }

 public async Task<(bool Success, string? Error)> CreateUserAsync(RegisterUserViewModel model, string avatarUrl)
 {
 try
 {
 // Set authorization header using client credentials token via ITokenAccessor
 try
 {
 await _tokenAccessor.SetAuthorizationHeaderAsync(_http, isClient: true);
 }
 catch (Exception ex)
 {
 return (false, "Failed to acquire client token: " + ex.Message);
 }

 // create user representation
 var usersUrl = $"{_kc.Host.TrimEnd('/')}/admin/realms/{_kc.Realm}/users";

 var createUser = new
 {
 attributes = new Dictionary<string, IEnumerable<string>> { { "avatar", new[] { avatarUrl } } },
 username = model.Email,
 email = model.Email,
 enabled = true,
 emailVerified = true,
 credentials = new[] { new { temporary = false, type = "password", value = model.Password } }
 };

 var serializerOptions = new JsonSerializerOptions { PropertyNamingPolicy = JsonNamingPolicy.CamelCase };
 var uJson = JsonSerializer.Serialize(createUser, serializerOptions);
 var resp = await _http.PostAsync(usersUrl, new StringContent(uJson, Encoding.UTF8, "application/json"));
 if (!resp.IsSuccessStatusCode)
 {
 var err = await resp.Content.ReadAsStringAsync();
 return (false, err);
 }

 return (true, null);
 }
 catch (Exception ex)
 {
 return (false, ex.Message);
 }
 }
 }
}
