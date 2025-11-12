using System;
using Microsoft.AspNetCore.Http;

namespace Lab1.Extensions
{
    public static class HttpRequestExtensions
    {
        public static bool IsAjaxRequest(this HttpRequest request)
        {
            if (request == null) throw new ArgumentNullException(nameof(request));
            return string.Equals(request.Headers["x-requested-with"], "xmlhttprequest",
                                 StringComparison.OrdinalIgnoreCase);
        }
    }
}
