using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;

namespace Lab1.Services.FileService
{
    public class LocalFileService : IFileService
    {
        private readonly IWebHostEnvironment _env;

        public LocalFileService(IWebHostEnvironment env) => _env = env;

        public async Task<string> SaveFileAsync(IFormFile file)
        {
            // Generate filename
            var ext = Path.GetExtension(file.FileName);
            if (string.IsNullOrWhiteSpace(ext)) ext = ".png";
            var fileName = $"{Guid.NewGuid():N}{ext}";

            // Ensure path exists
            var webRoot = _env.WebRootPath ?? Path.Combine(Directory.GetCurrentDirectory(), "wwwroot");
            var imagesDir = Path.Combine(webRoot, "images");
            if (!Directory.Exists(imagesDir)) Directory.CreateDirectory(imagesDir);

            var fullPath = Path.Combine(imagesDir, fileName);
            await using (var fs = new FileStream(fullPath, FileMode.Create))
            {
                await file.CopyToAsync(fs);
            }
            // web url:
            return $"/images/{fileName}";
        }
    }
}
