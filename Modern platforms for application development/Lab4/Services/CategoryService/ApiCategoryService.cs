using System.Text.Json;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Services.CategoryService;

namespace Lab1.Services;

public class ApiCategoryService : ICategoryService
{
    private readonly HttpClient _httpClient;
    private readonly JsonSerializerOptions _serializerOptions;
    private readonly ILogger<ApiCategoryService> _logger;

    public ApiCategoryService(HttpClient httpClient, ILogger<ApiCategoryService> logger)
    {
        _httpClient = httpClient;
        _logger = logger;
        _serializerOptions = new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };
    }

    public async Task<ResponseData<List<Category>>> GetCategoryListAsync()
    {
        var response = await _httpClient.GetAsync("");

        if (response.IsSuccessStatusCode)
        {
            try
            {
                var categories = await response.Content.ReadFromJsonAsync<List<Category>>(_serializerOptions);
                return ResponseData<List<Category>>.Success(categories ?? new List<Category>());
            }
            catch (JsonException ex)
            {
                _logger.LogError($"Ошибка десериализации: {ex.Message}");
                return ResponseData<List<Category>>.Error($"Ошибка: {ex.Message}");
            }
        }

        _logger.LogError($"Данные не получены от сервера. Error: {response.StatusCode}");
        return ResponseData<List<Category>>.Error($"Данные не получены от сервера. Error: {response.StatusCode}");
    }
}