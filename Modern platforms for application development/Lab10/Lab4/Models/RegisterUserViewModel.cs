using System.ComponentModel.DataAnnotations;
using Microsoft.AspNetCore.Http;

namespace Lab1.Models
{
    public class RegisterUserViewModel
    {
        [Required, EmailAddress]
        public string Email { get; set; } = string.Empty;

        [Required, DataType(DataType.Password)]
        public string Password { get; set; } = string.Empty;

        [Required, DataType(DataType.Password)]
        [Compare(nameof(Password))]
        public string ConfirmPassword { get; set; } = string.Empty;

        public IFormFile? Avatar { get; set; }
    }
}
