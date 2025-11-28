using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using System.Net.Http.Json;
using System.Text;
using Microsoft.AspNetCore.Components.WebAssembly.Authentication;

namespace Lab1BWA.Services
{
    public class DataService : IDataService
    {
        private readonly HttpClient _httpClient;
        private readonly IConfiguration _configuration;
        private readonly IAccessTokenProvider _tokenProvider;
        private string _pageSize = "3";
        private string? _apiBaseUrl;

        public event Action? DataLoaded;

        public List<Category> Categories { get; set; } = new();
        public List<Car> Cars { get; set; } = new();
        public bool Success { get; set; }
        public string ErrorMessage { get; set; } = string.Empty;
        public int TotalPages { get; set; }
        public int CurrentPage { get; set; }
        public Category? SelectedCategory { get; set; }

        public DataService(HttpClient httpClient, IConfiguration configuration, IAccessTokenProvider tokenProvider)
        {
            _httpClient = httpClient;
            _configuration = configuration;
            _tokenProvider = tokenProvider;

            // Get configuration from appsettings
            _apiBaseUrl = _configuration["ApiSettings:ApiBaseUrl"];
            var pageSizeConfig = _configuration["ApiSettings:PageSize"];
            if (!string.IsNullOrWhiteSpace(pageSizeConfig))
            {
                _pageSize = pageSizeConfig;
            }
        }

        public async Task GetCarListAsync(int pageNo = 1)
        {
            try
            {
                // Request JWT token
                var tokenRequest = await _tokenProvider.RequestAccessToken();
                if (!tokenRequest.TryGetToken(out var token))
                {
                    Success = false;
                    ErrorMessage = "Failed to obtain access token";
                    Cars = new();
                    DataLoaded?.Invoke();
                    return;
                }

                var route = new StringBuilder("cars/");

                // Add category to route
                if (SelectedCategory is not null && !string.IsNullOrWhiteSpace(SelectedCategory.NormalizedName))
                {
                    route.Append($"{SelectedCategory.NormalizedName}/");
                }

                // Build query parameters
                var queryParams = new List<string>();

                // Add page number
                if (pageNo > 1)
                {
                    queryParams.Add($"pageNo={pageNo}");
                }

                // Add page size
                if (!_pageSize.Equals("3"))
                {
                    queryParams.Add($"pageSize={_pageSize}");
                }

                // Add query string to URL
                if (queryParams.Count > 0)
                {
                    route.Append("?");
                    route.Append(string.Join("&", queryParams));
                }

                // Build full URL - explicitly combine BaseAddress with relative URL
                var baseAddress = _httpClient.BaseAddress?.ToString().TrimEnd('/') ?? "http://localhost:5002/api";
                var fullUrl = $"{baseAddress}/{route}";
                Console.WriteLine($"[DataService] Full URL for cars: {fullUrl}");

                // Add JWT token to default headers
                _httpClient.DefaultRequestHeaders.Authorization =
                    new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token.Value);

                var response = await _httpClient.GetAsync(fullUrl);

                if (response.IsSuccessStatusCode)
                {
                    var content = await response.Content.ReadFromJsonAsync<ResponseData<ListModel<Car>>>();
                    if (content?.Successfull == true && content.Data != null)
                    {
                        Cars = content.Data.Items ?? new();
                        TotalPages = content.Data.TotalPages;
                        CurrentPage = content.Data.CurrentPage;
                        Success = true;
                        ErrorMessage = string.Empty;
                    }
                    else
                    {
                        Success = false;
                        ErrorMessage = content?.ErrorMessage ?? "Invalid response format";
                        Cars = new();
                    }
                }
                else
                {
                    Success = false;
                    ErrorMessage = $"Server error: {response.StatusCode} - {response.ReasonPhrase}";
                    Cars = new();
                }

                // Remove Authorization header after request
                _httpClient.DefaultRequestHeaders.Authorization = null;
            }
            catch (Exception ex)
            {
                Success = false;
                ErrorMessage = $"Error: {ex.Message}";
                Cars = new();
                Console.WriteLine($"Error loading cars: {ex}");

            }
            finally
            {
                DataLoaded?.Invoke();
            }
        }

        public async Task GetCategoryListAsync()
        {
            try
            {
                // Request JWT token
                var tokenRequest = await _tokenProvider.RequestAccessToken();
                if (!tokenRequest.TryGetToken(out var token))
                {
                    Success = false;
                    ErrorMessage = "Failed to obtain access token";
                    Categories = new();
                    DataLoaded?.Invoke();
                    return;
                }

                // Build full URL - explicitly combine BaseAddress with relative URL
                var baseAddress = _httpClient.BaseAddress?.ToString().TrimEnd('/') ?? "http://localhost:5002/api";
                var fullUrl = $"{baseAddress}/categories";
                Console.WriteLine($"[DataService] Full URL: {fullUrl}");

                // Add JWT token to default headers
                _httpClient.DefaultRequestHeaders.Authorization =
                    new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token.Value);

                var response = await _httpClient.GetAsync(fullUrl);

                Console.WriteLine($"[DataService] Categories request: {response.StatusCode} - {response.ReasonPhrase}");

                if (response.IsSuccessStatusCode)
                {
                    var content = await response.Content.ReadFromJsonAsync<List<Category>>();
                    if (content != null)
                    {
                        Categories = content;
                        Success = true;
                        ErrorMessage = string.Empty;
                        Console.WriteLine($"[DataService] Loaded {Categories.Count} categories");
                    }
                    else
                    {
                        Success = false;
                        ErrorMessage = "Invalid response format from server";
                        Categories = new();
                    }
                }
                else
                {
                    Success = false;
                    ErrorMessage = $"Server error: {response.StatusCode} - {response.ReasonPhrase}";
                    Categories = new();
                    Console.WriteLine($"[DataService] Error response body: {await response.Content.ReadAsStringAsync()}");
                }

                // Remove Authorization header after request
                _httpClient.DefaultRequestHeaders.Authorization = null;
            }
            catch (Exception ex)
            {
                Success = false;
                ErrorMessage = $"Error: {ex.Message}";
                Categories = new();
                Console.WriteLine($"Error loading categories: {ex}");
            }
            finally
            {
                DataLoaded?.Invoke();
            }
        }
    }
}
