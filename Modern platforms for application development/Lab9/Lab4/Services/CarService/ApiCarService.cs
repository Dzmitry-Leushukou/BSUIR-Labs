using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Microsoft.AspNetCore.Http;
using Lab1.Services.Authentication;
using Lab1.Services.CarService;
using Microsoft.Extensions.Logging;

namespace Lab1.Services
{
    public class ApiCarService : ICarService
    {
        private readonly HttpClient _httpClient;
        private readonly ILogger<ApiCarService> _logger;
        private readonly JsonSerializerOptions _serializerOptions = new() { PropertyNameCaseInsensitive = true };
        private readonly string _pageSize = "3";
        private readonly ITokenAccessor _tokenAccessor;

        public ApiCarService(HttpClient httpClient, ILogger<ApiCarService> logger, IConfiguration config, ITokenAccessor tokenAccessor)
        {
            _httpClient = httpClient;
            _logger = logger;
            _tokenAccessor = tokenAccessor;
            _pageSize = (config["PageSettings:ItemsPerPage"] ?? "3");
        }

        public async Task<ResponseData<ListModel<Car>>> GetCarListAsync(string? categoryNormalizedName, int pageNo = 1)
        {
            try
            {
                var url = new StringBuilder();
                if (!string.IsNullOrWhiteSpace(categoryNormalizedName))
                    url.Append($"{categoryNormalizedName}");
                url.Append($"?pageNo={pageNo}&pageSize={_pageSize}");

                var response = await _httpClient.GetAsync(url.ToString());
                if (!response.IsSuccessStatusCode)
                    return ResponseData<ListModel<Car>>.Error($"Ошибка {response.StatusCode}");

                var data = await response.Content.ReadFromJsonAsync<ResponseData<ListModel<Car>>>(_serializerOptions);
                return data ?? ResponseData<ListModel<Car>>.Error("Empty response");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Get list failed");
                return ResponseData<ListModel<Car>>.Error(ex.Message);
            }
        }

        public async Task<ResponseData<Car>> GetCarByIdAsync(int id)
        {
            try
            {
                var response = await _httpClient.GetAsync($"{id}");
                if (!response.IsSuccessStatusCode)
                    return ResponseData<Car>.Error($"Ошибка {response.StatusCode}");

                var data = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_serializerOptions);
                return data ?? ResponseData<Car>.Error("Empty response");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Get by id failed");
                return ResponseData<Car>.Error(ex.Message);
            }
        }

        public async Task<ResponseData<Car>> CreateCarAsync(Car car, IFormFile? formFile)
        {
            try
            {
                await _tokenAccessor.SetAuthorizationHeaderAsync(_httpClient, isClient: false);
            }
            catch (Exception e)
            {
                return ResponseData<Car>.Error($"Объект не добавлен. Error: {e.Message}");
            }

            using var content = new MultipartFormDataContent();
            var json = JsonSerializer.Serialize(car);
            content.Add(new StringContent(json, Encoding.UTF8, "application/json"), "car");

            if (formFile != null)
            {
                var streamContent = new StreamContent(formFile.OpenReadStream());
                content.Add(streamContent, "file", formFile.FileName);
            }

            var response = await _httpClient.PostAsync("", content);
            if (!response.IsSuccessStatusCode)
                return ResponseData<Car>.Error($"Ошибка {response.StatusCode}");

            var result = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_serializerOptions);
            return result ?? ResponseData<Car>.Error("Empty response");
        }

        public async Task UpdateCarAsync(int id, Car car, IFormFile? formFile)
        {
            await _tokenAccessor.SetAuthorizationHeaderAsync(_httpClient, isClient: false);

            using var content = new MultipartFormDataContent();
            var json = JsonSerializer.Serialize(car);
            content.Add(new StringContent(json, Encoding.UTF8, "application/json"), "car");

            if (formFile != null)
            {
                var streamContent = new StreamContent(formFile.OpenReadStream());
                content.Add(streamContent, "file", formFile.FileName);
            }

            var response = await _httpClient.PutAsync($"{id}", content);
            response.EnsureSuccessStatusCode();
        }

        public async Task DeleteCarAsync(int id)
        {
            await _tokenAccessor.SetAuthorizationHeaderAsync(_httpClient, isClient: false);
            var response = await _httpClient.DeleteAsync($"{id}");
            response.EnsureSuccessStatusCode();
        }
    }
}
