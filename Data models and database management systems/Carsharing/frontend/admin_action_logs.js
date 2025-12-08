// Функция для загрузки и отображения данных таблицы Action logs
async function loadActionLogs() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем все логи действий (с большим лимитом)
        const response = await fetch('/action_logs/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const actionLogs = await response.json();
            displayActionLogs(actionLogs);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Action logs: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Action logs:', error);
        alert('Ошибка при загрузке Action logs');
    }
}

// Функция для отображения данных в таблице Action logs
function displayActionLogs(actionLogs) {
    const tableBody = document.getElementById('action-logs-table-body');
    tableBody.innerHTML = '';
    
    actionLogs.forEach(log => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${log.id}</td>
            <td>${log.actor_user_id}</td>
            <td>${log.action_type}</td>
            <td>${log.target_user_id || ''}</td>
            <td>${log.target_car_id || ''}</td>
            <td>${log.target_rental_id || ''}</td>
            <td>${log.description || ''}</td>
            <td>${JSON.stringify(log.old_values) || ''}</td>
            <td>${JSON.stringify(log.new_values) || ''}</td>
            <td>${log.user_agent || ''}</td>
            <td>${new Date(log.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
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
    loadActionLogs();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterActionLogs);
    });
});