// Глобальные переменные для пагинации
let currentMaintenanceRequestPage = 0;
const maintenanceRequestsPerPage = 10;

// Функция для загрузки и отображения данных таблицы Maintenance requests с пагинацией
async function loadMaintenanceRequests(page = 0) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentMaintenanceRequestPage = validPageNum;
    const offset = validPageNum * maintenanceRequestsPerPage;
    
    try {
        // Загружаем запросы на обслуживание с пагинацией
        const response = await fetch(`/maintenance_requests/?offset=${offset}&limit=${maintenanceRequestsPerPage}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const maintenanceRequests = await response.json();
            displayMaintenanceRequests(maintenanceRequests);
            setupMaintenanceRequestPagination(page);
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

// Функция для настройки пагинации запросов на обслуживание
function setupMaintenanceRequestPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество запросов на обслуживание для определения количества страниц
    getMaintenanceRequestsCount().then(totalCount => {
        const totalPages = Math.ceil(totalCount / maintenanceRequestsPerPage);
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('maintenance-requests-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'maintenance-requests-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#maintenance-requests-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'maintenance-requests-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#maintenance-requests-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#maintenance-requests-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#maintenance-requests-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadMaintenanceRequests(${safePageNum - 1})">Предыдущая</button>`;
        }
        
        // Кнопки страниц
        const maxVisiblePages = 5;
        let startPage = Math.max(0, safePageNum - Math.floor(maxVisiblePages / 2));
        let endPage = Math.min(totalPages - 1, startPage + maxVisiblePages - 1);
        
        if (endPage - startPage + 1 < maxVisiblePages) {
            startPage = Math.max(0, endPage - maxVisiblePages + 1);
        }
        
        for (let i = startPage; i <= endPage; i++) {
            if (i === safePageNum) {
                paginationHTML += `<button class="pagination-btn active">${i + 1}</button>`;
            } else {
                paginationHTML += `<button class="pagination-btn" onclick="loadMaintenanceRequests(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadMaintenanceRequests(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    });
}

// Функция для получения общего количества запросов на обслуживание
async function getMaintenanceRequestsCount() {
    try {
        const response = await fetch('/maintenance_requests/count', {
            method: 'GET',
            headers: {
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const countData = await response.json();
            return countData.count || 0;
        } else {
            return 0;
        }
    } catch (error) {
        console.error('Ошибка при получении количества запросов на обслуживание:', error);
        return 0;
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
    loadMaintenanceRequests(0);  // Загружаем первую страницу
    
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