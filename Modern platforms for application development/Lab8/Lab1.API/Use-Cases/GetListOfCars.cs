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

        var query = _db.Cars.Include(c => c.Category).AsQueryable(); // исправлено на c

        // Фильтрация по категории, если указана
        if (!string.IsNullOrEmpty(request.categoryNormalizedName))
        {
            query = query.Where(c => c.Category.NormalizedName == request.categoryNormalizedName); // исправлено на c
        }

        // Получаем общее количество записей для пагинации
        var totalCount = await query.CountAsync(cancellationToken);
        var totalPages = (int)Math.Ceiling(totalCount / (double)request.pageSize);

        // Применяем пагинацию
        var cars = await query // исправлено на cars
            .Skip((request.pageNo - 1) * request.pageSize)
            .Take(request.pageSize)
            .ToListAsync(cancellationToken);

        // Создаем модель списка
        var listModel = new ListModel<Car>
        {
            Items = cars, // исправлено на cars
            CurrentPage = request.pageNo,
            TotalPages = totalPages
        };

        return ResponseData<ListModel<Car>>.Success(listModel);
    }
}