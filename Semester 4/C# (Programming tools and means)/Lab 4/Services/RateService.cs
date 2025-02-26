using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;
using Lab_1.Entities;
using System.Text.Json.Serialization;
using System.Net.Http.Json;
using Newtonsoft.Json;


namespace Lab_1.Services
{
    internal class RateService:IRateService
    {
       HttpClient _httpClient;
        public RateService(HttpClient httpClient)
        {
            _httpClient = httpClient;
        }
    
        public async Task<IEnumerable<Rate>> GetRatesAsync(DateTime date)
        {
            var response = await _httpClient.GetAsync($"https://api.nbrb.by/exrates/rates?ondate={date.ToString("yyyy-MM-dd")}&periodicity=0");
            if(response.IsSuccessStatusCode)
            {
                return JsonConvert.DeserializeObject<IEnumerable<Rate>>(await response.Content.ReadAsStringAsync());
            }
            else
            {
                return Enumerable.Empty<Rate>();
            }
                
        }
    }
}
