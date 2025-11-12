using System.Net.Http;

namespace Lab1.Services.Authentication
{
    public interface ITokenAccessor
    {
        /// <summary>
        /// Добавляет в переданный HttpClient заголовок Authorization: Bearer.
        /// Если isClient = true — берём client-credentials токен;
        /// иначе — access_token текущего пользователя (через OIDC).
        /// </summary>
        Task SetAuthorizationHeaderAsync(HttpClient httpClient, bool isClient);
    }
}
