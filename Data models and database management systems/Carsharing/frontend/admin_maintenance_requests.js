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
            // Maintenance requests are already sorted by backend (unresolved first, then resolved), so just display them
            displayMaintenanceRequests(maintenanceRequests);
        } else {
            const errorData = await response.json();
            let errorMessage = 'Неизвестная ошибка';
            if (errorData && typeof errorData === 'object') {
                if (errorData.detail) {
                    errorMessage = errorData.detail;
                } else if (errorData.message) {
                    errorMessage = errorData.message;
                } else {
                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                }
            } else {
                errorMessage = errorData || 'Неизвестная ошибка';
            }
            alert(`Ошибка при загрузке Maintenance requests: ${errorMessage}`);
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
        const statusText = request.status === 'open' ? 'Открыт' : request.status === 'resolved' ? 'Решен' : request.status;
        
        let actionButton = '';
        if (request.status === 'open') {
            actionButton = `<button class="btn btn-primary resolve-btn" data-request-id="${request.id}">Решён</button>`;
        } else {
            actionButton = ''; // Для закрытых заявок кнопка не отображается
        }
        
        row.innerHTML = `
            <td>${request.id}</td>
            <td>${request.car_id}</td>
            <td>${request.reported_by || ''}</td>
            <td>${new Date(request.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${request.resolved_at ? new Date(request.resolved_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' }) : ''}</td>
            <td class="status-cell" data-status="${request.status}" data-request-id="${request.id}">${statusText}</td>
            <td>${request.description}</td>
            <td>${actionButton}</td>
        `;
        tableBody.appendChild(row);
    });
    
    // Добавляем обработчики событий для кнопок "Решён"
    document.querySelectorAll('.resolve-btn').forEach(button => {
        button.addEventListener('click', function() {
            const requestId = parseInt(this.getAttribute('data-request-id'));
            resolveMaintenanceRequest(requestId, this);
        });
    });
}

// Функция для обновления статуса заявки на "Решена"
async function resolveMaintenanceRequest(requestId, buttonElement) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите отметить эту заявку как решённую?')) {
        return;
    }
    
    try {
        const response = await fetch(`/maintenance_requests/${requestId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                status: 'resolved',
                resolved_at: new Date().toISOString()
            })
        });
        
        if (response.ok) {
            const requestData = await response.json();
            
            // Обновляем статус в таблице
            const statusCell = document.querySelector(`.status-cell[data-request-id="${requestId}"]`);
            if (statusCell) {
                statusCell.textContent = 'Решен';
                statusCell.setAttribute('data-status', 'resolved');
            }
            
            // Удаляем кнопку "Решён"
            buttonElement.remove();
            
            alert('Заявка успешно отмечена как решённая');
        } else {
            const errorData = await response.json();
            let errorMessage = 'Неизвестная ошибка';
            if (errorData && typeof errorData === 'object') {
                if (errorData.detail) {
                    errorMessage = errorData.detail;
                } else if (errorData.message) {
                    errorMessage = errorData.message;
                } else {
                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                }
            } else {
                errorMessage = errorData || 'Неизвестная ошибка';
            }
            alert(`Ошибка при обновлении статуса заявки: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении статуса заявки:', error);
        alert('Ошибка при обновлении статуса заявки');
    }
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

// Функция для получения простого сообщения из объекта ошибки
function getSimpleMessage(obj) {
    // Проверяем, является ли объект простым объектом с сообщением
    if (obj && typeof obj === 'object' && !Array.isArray(obj)) {
        // Ищем возможные поля с сообщениями об ошибках
        if (obj.message) return obj.message;
        if (obj.msg) return obj.msg;
        if (obj.detail) return obj.detail;
        if (obj.error) return obj.error;
        
        // Если объект имеет только одно свойство, которое является строкой, возвращаем его
        const keys = Object.keys(obj);
        if (keys.length === 1 && typeof obj[keys[0]] === 'string') {
            return obj[keys[0]];
        }
    }
    return null; // Возвращаем null, если не удалось извлечь простое сообщение
}