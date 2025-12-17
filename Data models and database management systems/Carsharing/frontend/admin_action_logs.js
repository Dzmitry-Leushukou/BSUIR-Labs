// Глобальные переменные для пагинации
let currentActionLogPage = 0;
const actionLogsPerPage = 10;

// Функция для загрузки и отображения данных таблицы Action logs с пагинацией
async function loadActionLogs(page = 0) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentActionLogPage = validPageNum;
    const offset = validPageNum * actionLogsPerPage;
    
    try {
        // Загружаем логи действий с пагинацией
        const response = await fetch(`/action_logs/?offset=${offset}&limit=${actionLogsPerPage}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const actionLogs = await response.json();
            displayActionLogs(actionLogs);
            setupActionLogPagination(page);
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
            alert(`Ошибка при загрузке логов действий: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке логов действий:', error);
        alert('Ошибка при загрузке логов действий');
    }
}

// Функция для настройки пагинации логов действий
function setupActionLogPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество логов действий для определения количества страниц
    getActionLogsCount().then(totalCount => {
        // Проверяем, что totalCount - валидное число
        const count = totalCount || 0;
        const totalPages = Math.ceil(count / actionLogsPerPage);
        
        // Use the validated page number throughout the function
        const safePageNum = isFinite(pageNum) ? pageNum : 0;
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('action-logs-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'action-logs-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#action-logs-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'action-logs-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#action-logs-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#action-logs-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#action-logs-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadActionLogs(${safePageNum - 1})">Предыдущая</button>`;
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
                paginationHTML += `<button class="pagination-btn" onclick="loadActionLogs(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadActionLogs(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    }).catch(error => {
        console.error('Ошибка при настройке пагинации:', error);
        // Создаем контейнер для пагинации даже если возникла ошибка при получении количества
        let paginationContainer = document.getElementById('action-logs-pagination');
        if (!paginationContainer) {
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'action-logs-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#action-logs-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'action-logs-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#action-logs-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#action-logs-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#action-logs-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        // Выводим кнопку обновления, если возникла ошибка
        paginationContainer.innerHTML = '<button class="pagination-btn" onclick="loadActionLogs(0)">Обновить</button>';
    });
}

// Функция для получения общего количества логов действий
async function getActionLogsCount() {
    try {
        const userId = localStorage.getItem('user_id');
        if (!userId) {
            throw new Error('Пользователь не авторизован');
        }
        
        const response = await fetch('/action_logs/count', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const countData = await response.json();
            return countData.count || 0;
        } else {
            const errorData = await response.json();
            console.error('Ошибка сервера при получении количества логов действий:', errorData);
            return 0;
        }
    } catch (error) {
        console.error('Ошибка при получении количества логов действий:', error);
        return 0;
    }
}

// Функция для отображения данных в таблице Action logs
function displayActionLogs(actionLogs) {
    const tableBody = document.getElementById('action-logs-table-body');
    tableBody.innerHTML = '';
    
    actionLogs.forEach(log => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${log.actor_email || log.actor_user_id}</td>
            <td>${translateActionType(log.action_type)}</td>
            <td>${log.target_user_email || log.target_user_id || ''}</td>
            <td>${log.target_car_vin || log.target_car_id || ''}</td>
            <td>${log.target_rental_id || ''}</td>
            <td>${translateDescription(log.description || '')}</td>
            <td>${log.old_values && log.old_values !== null ? JSON.stringify(log.old_values) : ''}</td>
            <td>${log.new_values && log.new_values !== null ? JSON.stringify(log.new_values) : ''}</td>
            <td>${new Date(log.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для фильтрации данных таблицы Action logs
function filterActionLogs() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#action-logs-table-body tr');
    
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
    loadActionLogs(0);  // Загружаем первую страницу
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterActionLogs);
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