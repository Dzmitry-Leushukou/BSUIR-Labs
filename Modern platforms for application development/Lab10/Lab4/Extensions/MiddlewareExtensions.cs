using Lab1.Middleware;

namespace Lab1.Extensions
{
    /// <summary>
    /// Extension methods for middleware configuration
    /// </summary>
    public static class MiddlewareExtensions
    {
        /// <summary>
        /// Add ErrorResponseLoggingMiddleware to the application pipeline
        /// </summary>
        public static IApplicationBuilder UseErrorResponseLogging(this IApplicationBuilder builder)
        {
         return builder.UseMiddleware<ErrorResponseLoggingMiddleware>();
        }
    }
}
