using System.Net.Http.Headers;
using System.Text.Json;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Options;
using Microsoft.Extensions.Logging;

namespace Lab1.Services.Authentication
{
    public sealed class KeycloakTokenAccessor : ITokenAccessor
    {
        private readonly IHttpContextAccessor _contextAccessor;
        private readonly IOptions<KeycloakData> _options;
        private readonly HttpClient _httpClient;
        private readonly ILogger<KeycloakTokenAccessor> _logger;

        public KeycloakTokenAccessor(
            IHttpContextAccessor contextAccessor,
            IOptions<KeycloakData> options,
            HttpClient httpClient,
            ILogger<KeycloakTokenAccessor> logger)
        {
            _contextAccessor = contextAccessor;
            _options = options;
            _httpClient = httpClient;
            _logger = logger;
        }

        public async Task SetAuthorizationHeaderAsync(HttpClient httpClient, bool isClient)
        {
            string token;
            try
            {
                token = isClient
                    ? await GetClientTokenAsync()
                    : await GetUserTokenAsync();
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to get user token, attempting client credentials token as fallback.");
                // Try client token as fallback
                token = await GetClientTokenAsync();
            }

            if (string.IsNullOrWhiteSpace(token))
            {
                _logger.LogError("No token available to set Authorization header.");
                throw new InvalidOperationException("No token available to set Authorization header.");
            }

            // Do not log token itself. Log token length as an indicator.
            _logger.LogInformation("Setting Authorization header using token (length={Length})", token.Length);

            httpClient.DefaultRequestHeaders.Authorization =
                new AuthenticationHeaderValue("Bearer", token);
        }

        private async Task<string> GetUserTokenAsync()
        {
            var httpContext = _contextAccessor.HttpContext
                ?? throw new InvalidOperationException("No HttpContext.");

            if (!(httpContext.User?.Identity?.IsAuthenticated ?? false))
                throw new InvalidOperationException("Пользователь не авторизован.");

            // Try common ways to read the token from the authentication ticket
            string? token = null;

            //1) Default GetTokenAsync (uses current default authenticate scheme)
            try { token = await httpContext.GetTokenAsync("access_token"); } catch { }

            //2) Try cookie scheme explicitly
            if (string.IsNullOrWhiteSpace(token))
            {
                try { token = await httpContext.GetTokenAsync("AppCookie", "access_token"); } catch { }
            }

            //3) Try OpenID Connect scheme name (keycloak)
            if (string.IsNullOrWhiteSpace(token))
            {
                try { token = await httpContext.GetTokenAsync("keycloak", "access_token"); } catch { }
            }

            if (string.IsNullOrWhiteSpace(token))
            {
                // As a last resort, try to authenticate the cookie scheme and inspect the tokens
                try
                {
                    var authResult = await httpContext.AuthenticateAsync("AppCookie");
                    if (authResult?.Succeeded == true && authResult.Properties != null)
                    {
                        token = authResult.Properties.GetTokenValue("access_token");
                    }
                }
                catch { }
            }

            if (string.IsNullOrWhiteSpace(token))
                throw new InvalidOperationException("Не удалось получить access_token пользователя. Убедитесь, что токены сохранены в cookie (SaveTokens) и пользователь вошёл.");

            _logger.LogDebug("User access token found (length={Length}).", token.Length);
            return token!;
        }

        private async Task<string> GetClientTokenAsync()
        {
            var kc = _options.Value;

            var url = $"{kc.Host.TrimEnd('/')}/realms/{kc.Realm}/protocol/openid-connect/token";
            using var content = new FormUrlEncodedContent(new[]
            {
                new KeyValuePair<string,string>("client_id", kc.ClientId),
                new KeyValuePair<string,string>("grant_type","client_credentials"),
                new KeyValuePair<string,string>("client_secret", kc.ClientSecret),
            });

            var response = await _httpClient.PostAsync(url, content);
            var body = await response.Content.ReadAsStringAsync();

            if (!response.IsSuccessStatusCode)
            {
                _logger.LogError("Client credentials token request failed: {Status} {Body}", response.StatusCode, body);
                throw new HttpRequestException($"Token endpoint returned {(int)response.StatusCode}: {body}");
            }

            using var doc = JsonDocument.Parse(body);
            var token = doc.RootElement.GetProperty("access_token").GetString();
            if (string.IsNullOrEmpty(token))
            {
                _logger.LogError("Client token response did not contain access_token: {Body}", body);
                throw new InvalidOperationException("access_token not found in response.");
            }

            _logger.LogDebug("Client access token obtained (length={Length}).", token.Length);
            return token;
        }
    }
}
