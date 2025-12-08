// Функция для загрузки и отображения данных таблицы Maintenance requests
async function loadMaintenanceRequests() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем все запросы на обслуживание (с большим лимитом)
        const response = await fetch('/maintenance_requests/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const maintenanceRequests = await response.json();
            displayMaintenanceRequests(maintenanceRequests);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Maintenance requests: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Maintenance requests:', error);
        alert('Ошибка при загрузке Maintenance requests');
    }
}

// Функция для отображения данных в таблице Maintenance requests
function displayMaintenanceRequests(maintenanceRequests) {
    const tableBody = document.getElementById('maintenance-requests-table-body');
    tableBody.innerHTML = '';
    
    maintenanceRequests.forEach(request => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${request.id}</td>
            <td>${request.car_id}</td>
            <td>${request.reported_by || ''}</td>
            <td>${new Date(request.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${request.resolved_at ? new Date(request.resolved_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' }) : ''}</td>
            <td>${request.status}</td>
            <td>${request.description}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для фильтрации данных таблицы Maintenance requests
function filterMaintenanceRequests() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#maintenance-requests-table-body tr');
    
    rows.forEach(row => {
        let shouldShow = true;
        
        // Проверяем каждую ячейку в строке
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Проверяем, что ячейка существует
                if (i < row.cells.length) {
                    const cellValue = row.cells[i].textContent.trim();
                    if (!cellValue.toLowerCase().includes(filterValue.toLowerCase())) {
                        shouldShow = false;
                        break;
                    }
                }
            }
        }
        
        row.style.display = shouldShow ? '' : 'none';
    });
}

// Добавляем обработчики событий для фильтров
document.addEventListener('DOMContentLoaded', () => {
    loadMaintenanceRequests();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterMaintenanceRequests);
    });
});