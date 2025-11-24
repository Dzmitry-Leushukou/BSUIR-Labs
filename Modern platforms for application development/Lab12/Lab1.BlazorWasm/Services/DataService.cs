using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Microsoft.AspNetCore.Components.WebAssembly.Authentication;
using System.Net.Http.Json;
using System.Text;

namespace Lab1.BlazorWasm.Services
{
    public class DataService : IDataService
  {
        private readonly HttpClient _httpClient;
        private readonly IAccessTokenProvider _tokenProvider;
    private readonly IConfiguration _configuration;
        private string _pageSize = "10";
        private string _apiBaseUrl = "";

        public event Action? DataLoaded;

        public List<Category> Categories { get; set; } = new();
        public List<Car> Cars { get; set; } = new();
        public bool Success { get; set; } = false;
        public string ErrorMessage { get; set; } = string.Empty;
  public int TotalPages { get; set; } = 0;
   public int CurrentPage { get; set; } = 1;
        public Category? SelectedCategory { get; set; }

        public DataService(HttpClient httpClient, IAccessTokenProvider tokenProvider, IConfiguration configuration)
        {
  _httpClient = httpClient;
        _tokenProvider = tokenProvider;
            _configuration = configuration;
        _apiBaseUrl = _configuration["ApiBaseUrl"] ?? "https://localhost:7070";
            _pageSize = _configuration["PageSize"] ?? "10";
        }

      public async Task GetCarsListAsync(int pageNo = 1)
        {
      try
            {
     CurrentPage = pageNo;
      Success = false;
       ErrorMessage = string.Empty;

       var tokenRequest = await _tokenProvider.RequestAccessToken();
    if (!tokenRequest.TryGetToken(out var token))
             {
     ErrorMessage = "Не удалось получить токен авторизации";
   DataLoaded?.Invoke();
        return;
      }

      // Build URL
   var url = new StringBuilder($"{_apiBaseUrl}/api/cars/");

     // Add category if selected
      if (SelectedCategory is not null)
     {
      url.Append($"{SelectedCategory.NormalizedName}/");
       }

        // Add query parameters
         var queryParams = new List<string>();

      if (pageNo > 1)
        {
 queryParams.Add($"pageNo={pageNo}");
   }

         if (!_pageSize.Equals("10"))
    {
        queryParams.Add($"pageSize={_pageSize}");
       }

if (queryParams.Count > 0)
                {
         url.Append("?");
      url.Append(string.Join("&", queryParams));
    }

     _httpClient.DefaultRequestHeaders.Authorization = new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token.Value);

             var response = await _httpClient.GetAsync(url.ToString());

if (response.IsSuccessStatusCode)
      {
     var content = await response.Content.ReadFromJsonAsync<ResponseData<ListModel<Car>>>();
               if (content != null && content.Successfull)
  {
       Cars = content.Data?.Items ?? new();
           TotalPages = content.Data?.TotalPages ?? 1;
              Success = true;
              }
              else
           {
      ErrorMessage = content?.ErrorMessage ?? "Ошибка при получении данных";
        }
        }
                else
     {
      ErrorMessage = $"Ошибка сервера: {response.StatusCode}";
              }
     }
  catch (Exception ex)
    {
        ErrorMessage = $"Ошибка: {ex.Message}";
 }

 DataLoaded?.Invoke();
        }

      public async Task GetCategoryListAsync()
        {
          try
            {
       Success = false;
    ErrorMessage = string.Empty;

      var tokenRequest = await _tokenProvider.RequestAccessToken();
      if (!tokenRequest.TryGetToken(out var token))
       {
            ErrorMessage = "Не удалось получить токен авторизации";
    DataLoaded?.Invoke();
         return;
      }

     _httpClient.DefaultRequestHeaders.Authorization = new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token.Value);

            var response = await _httpClient.GetAsync($"{_apiBaseUrl}/api/categories");

          if (response.IsSuccessStatusCode)
         {
    var content = await response.Content.ReadFromJsonAsync<ResponseData<List<Category>>>();
        if (content != null && content.Successfull)
       {
     Categories = content.Data ?? new();
       Success = true;
                }
else
        {
    ErrorMessage = content?.ErrorMessage ?? "Ошибка при получении данных";
     }
    }
       else
     {
           ErrorMessage = $"Ошибка сервера: {response.StatusCode}";
  }
          }
            catch (Exception ex)
            {
                ErrorMessage = $"Ошибка: {ex.Message}";
       }

    DataLoaded?.Invoke();
        }
    }
}
