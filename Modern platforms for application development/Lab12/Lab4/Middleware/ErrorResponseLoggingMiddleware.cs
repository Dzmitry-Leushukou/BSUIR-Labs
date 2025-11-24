using Serilog;

namespace Lab1.Middleware
{
    /// <summary>
    /// Middleware for logging HTTP requests with non-2xx response status codes
    /// </summary>
    public class ErrorResponseLoggingMiddleware
    {
        private readonly RequestDelegate _next;
      private readonly ILogger<ErrorResponseLoggingMiddleware> _logger;

      public ErrorResponseLoggingMiddleware(RequestDelegate next, ILogger<ErrorResponseLoggingMiddleware> logger)
        {
       _next = next;
         _logger = logger;
        }

        public async Task InvokeAsync(HttpContext context)
        {
          // Store original body stream
  var originalBodyStream = context.Response.Body;

 try
            {
    // Only buffer if response hasn't started yet
         if (!context.Response.HasStarted)
       {
   using (var memoryStream = new MemoryStream())
    {
        context.Response.Body = memoryStream;

        // Call next middleware
             await _next(context);

           // Log non-2xx responses
     if (context.Response.StatusCode < 200 || context.Response.StatusCode >= 300)
        {
        var requestPath = context.Request.Path.ToString();
             Log.Information("---> request {RequestPath} returns {StatusCode}", requestPath, context.Response.StatusCode);
     }

    // Copy buffered content back to original stream
                  memoryStream.Position = 0;
           await memoryStream.CopyToAsync(originalBodyStream);
 }
         }
    else
          {
     // Response already started, just call next
          await _next(context);
                }
  }
     finally
    {
  context.Response.Body = originalBodyStream;
   }
      }
    }
}
