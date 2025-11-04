using MediatR;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;

namespace Lab1.API.UseCases
{
    public sealed record SaveImage(IFormFile File) : IRequest<string>;

    public class SaveImageHandler : IRequestHandler<SaveImage, string>
    {
        private readonly IWebHostEnvironment _env;
        private readonly IHttpContextAccessor _http;

        public SaveImageHandler(IWebHostEnvironment env, IHttpContextAccessor http)
        {
            _env = env;
            _http = http;
        }

        public async Task<string> Handle(SaveImage request, CancellationToken ct)
        {
            var ext = Path.GetExtension(request.File.FileName);
            var fileName = $"{Guid.NewGuid()}{ext}";

            var webRoot = _env.WebRootPath ?? Path.Combine(Directory.GetCurrentDirectory(), "wwwroot");
            var imagesFolder = Path.Combine(webRoot, "Images");
            Directory.CreateDirectory(imagesFolder);

            var fullPath = Path.Combine(imagesFolder, fileName);
            using (var fs = new FileStream(fullPath, FileMode.Create))
                await request.File.CopyToAsync(fs, ct);

            var baseUrl = _http.HttpContext is null
                ? string.Empty
                : $"{_http.HttpContext.Request.Scheme}://{_http.HttpContext.Request.Host}";

            // вернём абсолютный URL (UI удобно показывать)
            return $"{baseUrl}/Images/{fileName}";
        }
    }
}
