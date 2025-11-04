using System.Net.Http;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json;                  // ← важно: этот using нужен
using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Services.CarService;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;

namespace Lab1.Services // или Lab1.Services.CarService — подгони под свой проект
{
    public class ApiCarService : ICarService
    {
        private readonly HttpClient _httpClient;
        private readonly string _pageSize;
        private readonly JsonSerializerOptions _jsonOptions = new()
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,   // ← фикс: было JsonSerializerNamingPolicy
            PropertyNameCaseInsensitive = true
        };
        private readonly ILogger<ApiCarService> _logger;

        public ApiCarService(HttpClient httpClient, IConfiguration configuration, ILogger<ApiCarService> logger)
        {
            _httpClient = httpClient;
            _logger = logger;
            _pageSize = configuration.GetSection("PageSettings")["ItemsPerPage"] ?? "3";
        }

        public async Task<ResponseData<ListModel<Car>>> GetCarListAsync(string? categoryNormalizedName, int pageNo = 1)
        {
            var url = string.IsNullOrWhiteSpace(categoryNormalizedName)
                ? $"?pageNo={pageNo}&pageSize={_pageSize}"
                : $"{categoryNormalizedName}?pageNo={pageNo}&pageSize={_pageSize}";

            var response = await _httpClient.GetAsync(url);
            if (response.IsSuccessStatusCode)
            {
                var data = await response.Content.ReadFromJsonAsync<ResponseData<ListModel<Car>>>(_jsonOptions);
                return data ?? ResponseData<ListModel<Car>>.Error("Empty response");
            }

            _logger.LogError("List failed: {Code} {Reason}", (int)response.StatusCode, response.ReasonPhrase);
            return ResponseData<ListModel<Car>>.Error($"Error {(int)response.StatusCode}");
        }

        public async Task<ResponseData<Car>> GetCarByIdAsync(int id)
        {
            var response = await _httpClient.GetAsync($"{id}");
            if (response.IsSuccessStatusCode)
            {
                var data = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_jsonOptions);
                return data ?? ResponseData<Car>.Error("Empty response");
            }

            _logger.LogError("GetById failed: {Code} {Reason}", (int)response.StatusCode, response.ReasonPhrase);
            return ResponseData<Car>.Error($"Error {(int)response.StatusCode}");
        }

        public async Task DeleteCarAsync(int id)
        {
            var response = await _httpClient.DeleteAsync($"{id}");
            if (!response.IsSuccessStatusCode)
                _logger.LogError("Delete failed: {Code} {Reason}", (int)response.StatusCode, response.ReasonPhrase);
        }

        public async Task<ResponseData<Car>> CreateCarAsync(Car product, IFormFile? formFile)
        {
            using var content = new MultipartFormDataContent();

            var json = JsonSerializer.Serialize(product, _jsonOptions);
            content.Add(new StringContent(json, Encoding.UTF8, "application/json"), "car");

            if (formFile != null)
            {
                var streamContent = new StreamContent(formFile.OpenReadStream());
                content.Add(streamContent, "file", formFile.FileName);
            }

            var response = await _httpClient.PostAsync("", content);
            if (response.IsSuccessStatusCode)
            {
                var data = await response.Content.ReadFromJsonAsync<ResponseData<Car>>(_jsonOptions);
                return data ?? ResponseData<Car>.Error("Empty response");
            }

            _logger.LogError("Create failed: {Code} {Reason}", (int)response.StatusCode, response.ReasonPhrase);
            return ResponseData<Car>.Error($"Error {(int)response.StatusCode}");
        }

        public async Task UpdateCarAsync(int id, Car product, IFormFile? formFile)
        {
            using var content = new MultipartFormDataContent();

            var json = JsonSerializer.Serialize(product, _jsonOptions);
            content.Add(new StringContent(json, Encoding.UTF8, "application/json"), "car");

            if (formFile != null)
            {
                var streamContent = new StreamContent(formFile.OpenReadStream());
                content.Add(streamContent, "file", formFile.FileName);
            }

            var response = await _httpClient.PutAsync($"{id}", content);
            if (!response.IsSuccessStatusCode)
                _logger.LogError("Update failed: {Code} {Reason}", (int)response.StatusCode, response.ReasonPhrase);
        }
    }
}
