using Lab_1.Entities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SQLite;
using LoremNET;
using System.Text.RegularExpressions;

namespace Lab_1.Services
{
    class SQLiteService:IDbService
    {
        private SQLiteConnection db;
        public SQLiteService()
        {
            db = new SQLiteConnection("database.db");
            db.CreateTable<Route>();
            db.CreateTable<Attraction>();
        }
        public IEnumerable<Route> GetAllRoutes()
        {
            return db.Table<Route>().ToList();
        }
        public IEnumerable<Attraction> GetRouteAttractions(int id)
        {
            return db.Table<Attraction>().Where(t=>t.RouteID == id).ToList();
        }
        public void Init()
        {
            
            List<Route> routes = new List<Route>
            {
                new Route {Name = "First", Duration = 1256.76},
                new Route {Name = "Second", Duration = 1256},
                new Route {Name = "Third", Duration = 800}
            };
            db.DeleteAll<Route>();
            db.DeleteAll<Attraction>();
            db.InsertAll(routes);

            List<Attraction> attractions = new List<Attraction>();

            //Gen attractions
            int amount = routes.Count*Random.Shared.Next(5,10*routes.Count-4);
            for (int i = 0; i < amount; i++)
            {
                attractions.Add(new Attraction { Name = Lorem.Words(1), Rating = Lorem.Number(0, 5) + 0.25 * Lorem.Number(0, 4) });
            }

            //Distribute
            int ind = 0, add = 5;
            foreach (Attraction attraction in attractions)
            {
                attraction.RouteID = routes[ind].Id;
                add--;

                if(add == 0)
                {
                    ind = (ind + 1) % 3;
                    add = 5;
                }
            }

            db.InsertAll(attractions);
        }
    }
}
