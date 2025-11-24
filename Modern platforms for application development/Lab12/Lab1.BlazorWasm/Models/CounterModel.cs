namespace Lab1.BlazorWasm.Models
{
    using System.ComponentModel.DataAnnotations;

    public class CounterModel
    {
        [Range(1, 100, ErrorMessage = "Counter value must be between 1 and 100")]
  public int CounterValue { get; set; }
    }
}
