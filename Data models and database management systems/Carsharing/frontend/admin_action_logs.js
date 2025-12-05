// Функция для загрузки и отображения данных таблицы Action logs
async function loadActionLogs() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/action_logs/', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
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
            <td>${new Date(log.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadActionLogs);