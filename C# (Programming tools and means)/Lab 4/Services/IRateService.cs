using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;
using Lab_1.Entities;

namespace Lab_1.Services
{
    public interface IRateService
    {
        Task<IEnumerable<Rate>> GetRatesAsync(DateTime date);
    }
}
