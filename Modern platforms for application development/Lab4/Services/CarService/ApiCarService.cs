using System.Text;
using System.Text.Json;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Services.CarService;

namespace Lab1.Services;

public class ApiCarService : ICarService
{
    private readonly HttpClient _httpClient;
    private readonly string _pageSize;
    private readonly JsonSerializerOptions _serializerOptions;
    private readonly ILogger<ApiCarService> _logger;

    public ApiCarService(HttpClient httpClient, IConfiguration configuration, ILogger<ApiCarService> logger)
    {
        _httpClient = httpClient;
        _pageSize = configuration["PageSettings:ItemsPerPage"] ?? "3";
        _logger = logger;
        _serializerOptions = new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };
    }

    public async Task<ResponseData<ListModel<Car>>> GetCarListAsync(string? categoryNormalizedName, int pageNo = 1)
    {
        // Подготовка URL запроса
        var urlString = new StringBuilder($"{_httpClient.BaseAddress!.AbsoluteUri}");

        // Добавить категорию в маршрут
        if (!string.IsNullOrEmpty(categoryNormalizedName))
        {
            urlString.Append($"{categoryNormalizedName}/");
        }

        // Добавить номер страницы в маршрут
        if (pageNo > 1)
        {
            urlString.Append($"page{pageNo}");
        }

        // Добавить размер страницы в строку запроса
        if (!_pageSize.Equals("3"))
        {
            urlString.Append(QueryString.Create("pageSize", _pageSize));
        }

        // Отправить запрос к API
        var response = await _httpClient.GetAsync(new Uri(urlString.ToString()));

        if (response.IsSuccessStatusCode)
        {
            try
            {
                var responseData = await response.Content.ReadFromJsonAsync<ResponseData<ListModel<Car>>>(_serializerOptions);
                return responseData ?? ResponseData<ListModel<Car>>.Error("Не удалось прочитать данные");
            }
            catch (JsonException ex)
            {
                _logger.LogError($"Ошибка десериализации: {ex.Message}");
                return ResponseData<ListModel<Car>>.Error($"Ошибка: {ex.Message}");
            }
        }

        _logger.LogError($"Данные не получены от сервера. Error: {response.StatusCode}");
        return ResponseData<ListModel<Car>>.Error($"Данные не получены от сервера. Error: {response.StatusCode}");
    }

    public async Task<ResponseData<Car>> GetCarByIdAsync(int id)
    {
        var response = await _httpClient.GetAsync($"{id}");

        if (response.IsSuccessStatusCode)
        {
            try
            {
                var result = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_serializerOptions);
                return result ?? ResponseData<Car>.Error("Не удалось прочитать данные");
            }
            catch (JsonException ex)
            {
                _logger.LogError($"Ошибка десериализации: {ex.Message}");
                return ResponseData<Car>.Error($"Ошибка: {ex.Message}");
            }
        }

        _logger.LogError($"Данные не получены от сервера. Error: {response.StatusCode}");
        return ResponseData<Car>.Error($"Данные не получены от сервера. Error: {response.StatusCode}");
    }

    public async Task<ResponseData<Car>> CreateProductAsync(Car car, IFormFile? formFile)
    {
        // Обработка файла изображения
        if (formFile != null)
        {
            // Сохраняем файл и получаем путь
            car.ImagePath = await SaveFileAsync(formFile);
        }
        else
        {
            car.ImagePath = "Images/noimage.jpg";
        }

        var json = JsonSerializer.Serialize(car, _serializerOptions);
        var content = new StringContent(json, Encoding.UTF8, "application/json");

        var response = await _httpClient.PostAsync("", content);

        if (response.IsSuccessStatusCode)
        {
            var responseData = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_serializerOptions);
            return responseData ?? ResponseData<Car>.Error("Не удалось прочитать данные");
        }

        _logger.LogError($"Автомобиль не создан. Error: {response.StatusCode}");
        return ResponseData<Car>.Error($"Автомобиль не создан. Error: {response.StatusCode}");
    }

    public async Task UpdateCarAsync(int id, Car car, IFormFile? formFile)
    {
        // Обработка файла изображения
        if (formFile != null)
        {
            // Сохраняем файл и получаем путь
            car.ImagePath = await SaveFileAsync(formFile);
        }

        var json = JsonSerializer.Serialize(car, _serializerOptions);
        var content = new StringContent(json, Encoding.UTF8, "application/json");

        var response = await _httpClient.PutAsync($"{id}", content);

        if (!response.IsSuccessStatusCode)
        {
            _logger.LogError($"Автомобиль не обновлен. Error: {response.StatusCode}");
        }
    }

    public async Task DeleteCarAsync(int id)
    {
        var response = await _httpClient.DeleteAsync($"{id}");

        if (!response.IsSuccessStatusCode)
        {
            _logger.LogError($"Автомобиль не удален. Error: {response.StatusCode}");
        }
    }

    private async Task<string> SaveFileAsync(IFormFile formFile)
    {
        // Сохранение файла в папку wwwroot/images
        var fileName = Guid.NewGuid().ToString() + Path.GetExtension(formFile.FileName);
        var filePath = Path.Combine("wwwroot", "images", fileName);

        using (var stream = new FileStream(filePath, FileMode.Create))
        {
            await formFile.CopyToAsync(stream);
        }

        return $"images/{fileName}";
    }
}