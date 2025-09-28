using Lab1.Domain.Models; 
using MediatR;
using Microsoft.EntityFrameworkCore;
using Lab1.API.Data;
using Lab1.Domain.Entities;

namespace Lab1.API.UseCases;

public sealed record GetListOfCars(
    string? categoryNormalizedName,
    int pageNo = 1,
    int pageSize = 3) : IRequest<ResponseData<ListModel<Car>>>;

public class GetListOfCarsHandler : IRequestHandler<GetListOfCars, ResponseData<ListModel<Car>>>
{
    private readonly AppDbContext _db;
    private readonly int _maxPageSize = 20;

    public GetListOfCarsHandler(AppDbContext db)
    {
        _db = db;
    }

    public async Task<ResponseData<ListModel<Car>>> Handle(GetListOfCars request, CancellationToken cancellationToken)
    {
        // Проверка размера страницы
        if (request.pageSize > _maxPageSize)
            request = request with { pageSize = _maxPageSize };

        var query = _db.Cars.Include(d => d.Category).AsQueryable();

        // Фильтрация по категории, если указана
        if (!string.IsNullOrEmpty(request.categoryNormalizedName))
        {
            query = query.Where(d => d.Category.NormalizedName == request.categoryNormalizedName);
        }

        // Получаем общее количество записей для пагинации
        var totalCount = await query.CountAsync(cancellationToken);
        var totalPages = (int)Math.Ceiling(totalCount / (double)request.pageSize);

        // Применяем пагинацию
        var Cars = await query
            .Skip((request.pageNo - 1) * request.pageSize)
            .Take(request.pageSize)
            .ToListAsync(cancellationToken);

        // Создаем модель списка (используем инициализатор свойств)
        var listModel = new ListModel<Car>
        {
            Items = Cars,
            CurrentPage = request.pageNo,
            TotalPages = totalPages
        };

        // Возвращаем успешный результат
        return ResponseData<ListModel<Car>>.Success(listModel);
    }
}