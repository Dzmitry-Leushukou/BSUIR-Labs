using Lab1.Models;

namespace Lab1.Services.Authentication
{
 public interface IKeycloakAdminService
 {
 Task<(bool Success, string? Error)> CreateUserAsync(RegisterUserViewModel model, string avatarUrl);
 }
}
