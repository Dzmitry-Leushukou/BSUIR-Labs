using Lab1.Domain.Entities;
using Lab1.Domain.Models;

namespace Lab1.BlazorWasm.Services
{
    /// <summary>
    /// Интерфейс сервиса для работы с данными API
    /// </summary>
    public interface IDataService
    {
        /// <summary>
  /// Событие, генерируемое при изменении данных
        /// </summary>
        event Action? DataLoaded;

 /// <summary>
        /// Список категорий
        /// </summary>
    List<Category> Categories { get; set; }

    /// <summary>
 /// Список автомобилей
        /// </summary>
        List<Car> Cars { get; set; }

        /// <summary>
  /// Признак успешного ответа на запрос к Api
        /// </summary>
        bool Success { get; set; }

        /// <summary>
    /// Сообщение об ошибке
    /// </summary>
        string ErrorMessage { get; set; }

        /// <summary>
        /// Количество страниц списка
        /// </summary>
        int TotalPages { get; set; }

        /// <summary>
        /// Номер текущей страницы
/// </summary>
int CurrentPage { get; set; }

        /// <summary>
        /// Фильтр по категории
        /// </summary>
  Category? SelectedCategory { get; set; }

     /// <summary>
        /// Получение списка всех автомобилей
        /// </summary>
    /// <param name="pageNo">номер страницы списка</param>
        Task GetCarsListAsync(int pageNo = 1);

    /// <summary>
        /// Получение списка категорий
        /// </summary>
    Task GetCategoryListAsync();
    }
}
