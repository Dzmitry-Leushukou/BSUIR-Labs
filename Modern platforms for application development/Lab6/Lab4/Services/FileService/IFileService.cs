using Microsoft.AspNetCore.Http;

namespace Lab1.Services.FileService
{
    public interface IFileService
    {
        /// <summary>
        /// Save a file into wwwroot/Images and return a web-relative URL (e.g. "/images/xyz.png").
        /// </summary>
        Task<string> SaveFileAsync(IFormFile file);
    }
}
