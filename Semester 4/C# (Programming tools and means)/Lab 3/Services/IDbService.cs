using Lab_1.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_1.Services
{
    public interface IDbService
    {
        IEnumerable<Route> GetAllRoutes();
        IEnumerable<Attraction> GetRouteAttractions(int id);
        void Init();
    }

}
