using Lab1.Domain.Entities;
using Lab1.Domain.Models;

namespace Lab1BWA.Services
{
    public interface IDataService
  {
        /// <summary>
     /// Event raised when data is loaded
        /// </summary>
     event Action? DataLoaded;

        /// <summary>
        /// List of categories
  /// </summary>
        List<Category> Categories { get; set; }

     /// <summary>
 /// List of cars
        /// </summary>
        List<Car> Cars { get; set; }

        /// <summary>
    /// Indicates successful API response
        /// </summary>
        bool Success { get; set; }

        /// <summary>
        /// Error message if request failed
        /// </summary>
   string ErrorMessage { get; set; }

        /// <summary>
  /// Total number of pages
        /// </summary>
  int TotalPages { get; set; }

 /// <summary>
        /// Current page number
        /// </summary>
        int CurrentPage { get; set; }

        /// <summary>
        /// Category filter
   /// </summary>
        Category? SelectedCategory { get; set; }

        /// <summary>
        /// Get list of all cars
      /// </summary>
  /// <param name="pageNo">Page number</param>
        /// <returns></returns>
     Task GetCarListAsync(int pageNo = 1);

 /// <summary>
        /// Get list of all categories
  /// </summary>
        /// <returns></returns>
        Task GetCategoryListAsync();
    }
}
