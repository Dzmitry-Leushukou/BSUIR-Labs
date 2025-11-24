namespace Lab1.Services.CarService
{
    public class MemoryCarService : ICarService
    {
        List<Car> _cars;
        List<Category> _categories;
        private readonly IConfiguration _configuration;
        private readonly ICategoryService _categoryService;
        public MemoryCarService(IConfiguration config, ICategoryService categoryService)
        {
            _configuration = config;
            _categories = categoryService.GetCategoryListAsync().Result.Data;
            SetupData();
        }
        private void SetupData()
        {
            _cars = new List<Car>
            {
                new Car {
                    Id = 1,
                    Name = "BMW E34",
                    Description = "Легендарный немецкий седан",
                    Price = 15000,
                    ImagePath = "Images/bmw_e34.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("sedans"))
                },
                new Car {
                    Id = 2,
                    Name = "Mercedes W124",
                    Description = "Надежный немецкий седан",
                    Price = 12000,
                    ImagePath = "Images/mercedes_w124.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("sedans"))
                },

                new Car {
                    Id = 3,
                    Name = "Audi TT",
                    Description = "Стильное купе",
                    Price = 25000,
                    ImagePath = "Images/audi_tt.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("coupe"))
                },
                new Car {
                    Id = 4,
                    Name = "BMW M4",
                    Description = "Спортивное купе",
                    Price = 65000,
                    ImagePath = "Images/bmw_m4.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("coupe"))
                },

                new Car {
                    Id = 5,
                    Name = "Volvo V90",
                    Description = "Практичный универсал",
                    Price = 45000,
                    ImagePath = "Images/volvo_v90.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("universals"))
                },
                new Car {
                    Id = 6,
                    Name = "Audi A6 Avant",
                    Description = "Премиальный универсал",
                    Price = 52000,
                    ImagePath = "Images/audi_a6_avant.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("universals"))
                },
                new Car {
                    Id = 7,
                    Name = "Volkswagen Golf",
                    Description = "Классический хетчбэк",
                    Price = 22000,
                    ImagePath = "Images/vw_golf.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("hatchbacks"))
                },
                new Car {
                    Id = 8,
                    Name = "Ford Focus",
                    Description = "Популярный хетчбэк",
                    Price = 19000,
                    ImagePath = "Images/ford_focus.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("hatchbacks"))
                },

                new Car {
                    Id = 9,
                    Name = "Toyota Sienna",
                    Description = "Семейный минивэн",
                    Price = 32000,
                    ImagePath = "Images/toyota_sienna.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("minivans"))
                },
                new Car {
                    Id = 10,
                    Name = "Honda Odyssey",
                    Description = "Комфортный минивэн",
                    Price = 35000,
                    ImagePath = "Images/honda_odyssey.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("minivans"))
                },

                new Car {
                    Id = 11,
                    Name = "Mazda MX-5",
                    Description = "Легендарный родстер",
                    Price = 28000,
                    ImagePath = "Images/mazda_mx5.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("roadsters"))
                },
                new Car {
                    Id = 12,
                    Name = "Porsche 718 Boxster",
                    Description = "Спортивный родстер",
                    Price = 65000,
                    ImagePath = "Images/porsche_boxster.png",
                    Mime = "image/png",
                    Category = _categories.Find(c => c.NormalizedName.Equals("roadsters"))
                }
            };


        }

        public async Task<ResponseData<ListModel<Car>>> GetCarListAsync(string? categoryNormalizedName, int pageNo = 1)
        {
            var itemsPerPage = _configuration.GetValue<int>("PageSettings:ItemsPerPage");

            var filteredItems = _cars
                .Where(m => categoryNormalizedName == null || (m.Category != null && m.Category.NormalizedName.Equals(categoryNormalizedName)))
                .ToList();

            int totalItems = filteredItems.Count;
            int totalPages = (int)Math.Ceiling((double)totalItems / itemsPerPage);

            var pagedItems = filteredItems
                .Skip((pageNo - 1) * itemsPerPage)
                .Take(itemsPerPage)
                .ToList();

            var result = new ListModel<Car>
            {
                Items = pagedItems,
                CurrentPage = pageNo,
                TotalPages = totalPages
            };

            return ResponseData<ListModel<Car>>.Success(result);
        }
        public Task<ResponseData<Car>> GetCarByIdAsync(int id)
        {
            var car = _cars.FirstOrDefault(x => x.Id == id);
            return Task.FromResult(
                car != null
                ? ResponseData<Car>.Success(car)
                : ResponseData<Car>.Error("Not found")
            );
        }

        public Task<ResponseData<Car>> CreateCarAsync(Car product, IFormFile? formFile)
        {
            product.Id = _cars.Any() ? _cars.Max(x => x.Id) + 1 : 1;
            if (string.IsNullOrWhiteSpace(product.ImagePath))
                product.ImagePath = "Images/noimage.jpg"; // заглушка
            _cars.Add(product);
            return Task.FromResult(ResponseData<Car>.Success(product));
        }

        public Task UpdateCarAsync(int id, Car car, IFormFile? formFile)
        {
            var idx = _cars.FindIndex(x => x.Id == id);
            if (idx >= 0)
            {
                car.Id = id;           // чтобы ID не потерялся
                _cars[idx] = car;
            }
            return Task.CompletedTask;
        }

        public Task DeleteCarAsync(int id)
        {
            _cars.RemoveAll(x => x.Id == id);
            return Task.CompletedTask;
        }

    }
}
