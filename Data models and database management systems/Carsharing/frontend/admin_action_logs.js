// Глобальные переменные для пагинации и фильтров
let currentActionLogPage = 0;
const actionLogsPerPage = 10;

// Текущие фильтры
let currentFilters = {
    start_date: null,
    end_date: null,
    action_type: null,
    user_id: null
};

// Глобальное хранилище для всех логов (для отображения деталей)
let allActionLogs = [];

// Функция для загрузки и отображения данных таблицы Action logs с пагинацией
async function loadActionLogs(page = 0) {
    if (!isAuthenticated()) {
        alert('Пользователь не авторизован');
        return;
    }

    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentActionLogPage = validPageNum;
    const offset = validPageNum * actionLogsPerPage;

    try {
        // Build query parameters
        const params = new URLSearchParams();
        params.append('offset', offset);
        params.append('limit', actionLogsPerPage);
        
        // Add filters
        if (currentFilters.start_date) {
            params.append('start_date', currentFilters.start_date);
        }
        if (currentFilters.end_date) {
            params.append('end_date', currentFilters.end_date);
        }
        if (currentFilters.user_id) {
            params.append('user_id', currentFilters.user_id);
        }
        if (currentFilters.action_type) {
            params.append('action_type', currentFilters.action_type);
        }

        // Загружаем логи действий с пагинацией и фильтрами
        const response = await authenticatedFetch(`/action_logs/?${params.toString()}`);

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

// Функция для применения фильтров
function applyFilters() {
    const startDateInput = document.getElementById('start-date');
    const endDateInput = document.getElementById('end-date');
    const actionTypeSelect = document.getElementById('action-type-filter');
    const userIdInput = document.getElementById('user-id-filter');

    // Get values
    const startDate = startDateInput.value;
    const endDate = endDateInput.value;
    const actionType = actionTypeSelect.value;
    const userId = userIdInput.value;

    // Convert to ISO format for API
    currentFilters.start_date = startDate ? new Date(startDate).toISOString() : null;
    currentFilters.end_date = endDate ? new Date(endDate).toISOString() : null;
    currentFilters.action_type = actionType || null;
    currentFilters.user_id = userId ? parseInt(userId) : null;

    // Reload with first page
    loadActionLogs(0);
}

// Функция для сброса фильтров
function resetFilters() {
    const startDateInput = document.getElementById('start-date');
    const endDateInput = document.getElementById('end-date');
    const actionTypeSelect = document.getElementById('action-type-filter');
    const userIdInput = document.getElementById('user-id-filter');

    // Reset values
    startDateInput.value = '';
    endDateInput.value = '';
    actionTypeSelect.value = '';
    userIdInput.value = '';

    // Reset filters
    currentFilters = {
        start_date: null,
        end_date: null,
        action_type: null,
        user_id: null
    };

    // Reload
    loadActionLogs(0);
}

// Функция для настройки пагинации логов действий
function setupActionLogPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;

    // Build query parameters for count endpoint
    const countParams = new URLSearchParams();
    if (currentFilters.start_date) {
        countParams.append('start_date', currentFilters.start_date);
    }
    if (currentFilters.end_date) {
        countParams.append('end_date', currentFilters.end_date);
    }
    if (currentFilters.user_id) {
        countParams.append('user_id', currentFilters.user_id);
    }
    if (currentFilters.action_type) {
        countParams.append('action_type', currentFilters.action_type);
    }

    // Подсчитываем общее количество логов действий для определения количества страниц
    getActionLogsCount(countParams.toString()).then(totalCount => {
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
async function getActionLogsCount(queryString = '') {
    try {
        if (!isAuthenticated()) {
            throw new Error('Пользователь не авторизован');
        }

        const endpoint = queryString ? `/action_logs/count?${queryString}` : '/action_logs/count';
        const response = await authenticatedFetch(endpoint);

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

    // Сохраняем все логи для последующего отображения деталей
    allActionLogs = actionLogs || [];

    if (!actionLogs || actionLogs.length === 0) {
        tableBody.innerHTML = '<tr><td colspan="4" style="text-align: center;">Логи действий не найдены</td></tr>';
        return;
    }

    actionLogs.forEach(log => {
        const row = document.createElement('tr');
        row.className = 'log-row';
        row.onclick = () => showLogDetails(log.id || '');
        row.innerHTML = `
            <td>${new Date(log.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${log.actor_email || log.actor_user_id || ''}</td>
            <td>${translateActionType(log.action_type)}</td>
            <td>${translateDescription(log.description || '')}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Глобальное хранилище для текущих деталей лога
let currentLogDetails = null;

// Функция для показа деталей лога в модальном окне
function showLogDetails(logId) {
    // Ищем лог в сохраненных данных
    const log = allActionLogs.find(l => (l.id || '') === logId);
    
    if (!log) {
        alert('Лог не найден');
        return;
    }
    
    currentLogDetails = log;
    displayLogDetails(log);
    openModal();
}

// Функция для отображения деталей лога
function displayLogDetails(log) {
    const modalBody = document.getElementById('modal-body');
    
    let html = `
        <div class="log-detail-section">
            <h3>Основная информация</h3>
            <table class="detail-table">
                <tr>
                    <td class="detail-label">ID:</td>
                    <td>${log.id || 'N/A'}</td>
                </tr>
                <tr>
                    <td class="detail-label">Время:</td>
                    <td>${log.created_at || 'N/A'}</td>
                </tr>
                <tr>
                    <td class="detail-label">Email пользователя:</td>
                    <td>${log.actor_email || 'N/A'}</td>
                </tr>
                <tr>
                    <td class="detail-label">ID пользователя:</td>
                    <td>${log.actor_user_id || 'N/A'}</td>
                </tr>
                <tr>
                    <td class="detail-label">Тип действия:</td>
                    <td>${translateActionType(log.action_type)} (${log.action_type})</td>
                </tr>
                <tr>
                    <td class="detail-label">Описание:</td>
                    <td>${translateDescription(log.description || 'N/A')}</td>
                </tr>
            </table>
        </div>
    `;
    
    // Целевые объекты
    const hasTargetInfo = log.target_user_email || log.target_user_id || 
                          log.target_car_vin || log.target_car_id || 
                          log.target_rental_id;
    
    if (hasTargetInfo) {
        html += `
            <div class="log-detail-section">
                <h3>Целевые объекты</h3>
                <table class="detail-table">
        `;
        
        if (log.target_user_email) {
            html += `
                <tr>
                    <td class="detail-label">Email целевого пользователя:</td>
                    <td>${log.target_user_email}</td>
                </tr>
            `;
        }
        if (log.target_user_id) {
            html += `
                <tr>
                    <td class="detail-label">ID целевого пользователя:</td>
                    <td>${log.target_user_id}</td>
                </tr>
            `;
        }
        if (log.target_car_vin) {
            html += `
                <tr>
                    <td class="detail-label">VIN автомобиля:</td>
                    <td>${log.target_car_vin}</td>
                </tr>
            `;
        }
        if (log.target_car_id) {
            html += `
                <tr>
                    <td class="detail-label">ID автомобиля:</td>
                    <td>${log.target_car_id}</td>
                </tr>
            `;
        }
        if (log.target_rental_id) {
            html += `
                <tr>
                    <td class="detail-label">ID аренды:</td>
                    <td>${log.target_rental_id}</td>
                </tr>
            `;
        }
        
        html += `</table></div>`;
    }
    
    // Старые значения
    if (log.old_values && Object.keys(log.old_values).length > 0) {
        html += `
            <div class="log-detail-section">
                <h3>Старые значения</h3>
                <pre class="detail-json">${translateFieldNames(JSON.stringify(log.old_values, null, 2))}</pre>
            </div>
        `;
    }
    
    // Новые значения
    if (log.new_values && Object.keys(log.new_values).length > 0) {
        html += `
            <div class="log-detail-section">
                <h3>Новые значения</h3>
                <pre class="detail-json">${translateFieldNames(JSON.stringify(log.new_values, null, 2))}</pre>
            </div>
        `;
    }
    
    // Дополнительная информация
    const hasExtraInfo = log.user_agent || log.ip_address;
    if (hasExtraInfo) {
        html += `
            <div class="log-detail-section">
                <h3>Дополнительная информация</h3>
                <table class="detail-table">
        `;
        
        if (log.user_agent) {
            html += `
                <tr>
                    <td class="detail-label">User Agent:</td>
                    <td class="detail-text">${log.user_agent}</td>
                </tr>
            `;
        }
        if (log.ip_address) {
            html += `
                <tr>
                    <td class="detail-label">IP адрес:</td>
                    <td>${log.ip_address}</td>
                </tr>
            `;
        }
        
        html += `</table></div>`;
    }
    
    modalBody.innerHTML = html;
}

// Функция для открытия модального окна
function openModal() {
    const modal = document.getElementById('log-detail-modal');
    modal.style.display = 'block';
    document.body.style.overflow = 'hidden';
}

// Функция для закрытия модального окна
function closeModal() {
    const modal = document.getElementById('log-detail-modal');
    modal.style.display = 'none';
    document.body.style.overflow = '';
    currentLogDetails = null;
}

// Закрытие модального окна при клике вне его
window.onclick = function(event) {
    const modal = document.getElementById('log-detail-modal');
    if (event.target === modal) {
        closeModal();
    }
}

// Функция для применения переводов к заголовкам колонок
function applyColumnTranslations() {
    const spans = document.querySelectorAll('span[data-i18n]');
    spans.forEach(span => {
        const key = span.getAttribute('data-i18n');
        span.textContent = translateColumnHeader(key);
    });
}

// Добавляем обработчики событий для фильтров
document.addEventListener('DOMContentLoaded', () => {
    applyColumnTranslations();  // Применяем переводы к заголовкам колонок
    loadActionLogs(0);  // Загружаем первую страницу
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
