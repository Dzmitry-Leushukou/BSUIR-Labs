using System.ComponentModel.DataAnnotations;

namespace Lab1.Blazor.SSR.Models
{
/// <summary>
    /// Model for counter input validation
    /// </summary>
    public class CounterModel
    {
        [Range(1, 10, ErrorMessage = "The field value must be between 1 and 10.")]
        public int CounterValue { get; set; } = 0;
    }
}
