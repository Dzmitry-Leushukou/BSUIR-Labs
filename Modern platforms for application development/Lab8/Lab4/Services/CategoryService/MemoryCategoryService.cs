namespace Lab1.Services.CategoryService
{
    public class MemoryCategoryService : ICategoryService
    {
        public Task<ResponseData<List<Category>>> GetCategoryListAsync()
        {
            var categories = new List<Category>
            {
                new Category {Id=1, Name="Седаны", NormalizedName="sedans"},
                new Category {Id=2, Name="Купе", NormalizedName="coupe"},
                new Category {Id=3, Name="Универсалы", NormalizedName="universals"},
                new Category {Id=4, Name="Хетчбэки", NormalizedName="hatchbacks"},
                new Category {Id=5, Name="Минивэны", NormalizedName="minivans"},
                new Category {Id=6, Name="Родстеры", NormalizedName="roadsters"}
            };
            var result = ResponseData<List<Category>>.Success(categories);
            return Task.FromResult(result);
        }
    }
}
