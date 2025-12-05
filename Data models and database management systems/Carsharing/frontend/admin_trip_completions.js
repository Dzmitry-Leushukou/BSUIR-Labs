// Функция для загрузки и отображения данных таблицы Trip completions
async function loadTripCompletions() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/rentals/', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const rentals = await response.json();
            // Отображаем только завершенные поездки (status = 'completed') или активные, которые требуют завершения
            const tripCompletions = rentals.filter(rental => rental.status === 'active' || rental.status === 'completed');
            displayTripCompletions(tripCompletions);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке поездок: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке поездок:', error);
        alert('Ошибка при загрузке поездок');
    }
}

// Функция для отображения данных в таблице Trip completions
function displayTripCompletions(rentals) {
    const tableBody = document.getElementById('trip-completions-table-body');
    tableBody.innerHTML = '';
    
    rentals.forEach(rental => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${rental.id}</td>
            <td>${rental.user_id}</td>
            <td>${rental.car_id}</td>
            <td>${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
            <td>${rental.ended_at ? new Date(rental.ended_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' }) : ''}</td>
            <td>${rental.price} BYN</td>
            <td class="status-${rental.status}">${rental.status}</td>
            <td>
                <button class="btn action-btn approve-btn" onclick="confirmTrip(${rental.id})">Подтвердить</button>
                <button class="btn action-btn reject-btn" onclick="rejectTrip(${rental.id})">Отклонить</button>
                <button class="btn action-btn skip-btn" onclick="skipTrip(${rental.id})">Пропустить</button>
            </td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для подтверждения завершения поездки
async function confirmTrip(rentalId) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите подтвердить завершение поездки?')) {
        return;
    }
    
    try {
        const response = await fetch(`/rentals/${rentalId}`, {
            method: 'PUT',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: 'completed' })
        });
        
        if (response.ok) {
            alert('Поездка успешно подтверждена как завершенная');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при подтверждении поездки: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при подтверждении поездки:', error);
        alert('Ошибка при подтверждении поездки');
    }
}

// Функция для отклонения завершения поездки
async function rejectTrip(rentalId) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите отклонить завершение поездки?')) {
        return;
    }
    
    try {
        const response = await fetch(`/rentals/${rentalId}`, {
            method: 'PUT',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: 'active' }) // Возвращаем статус на активный
        });
        
        if (response.ok) {
            alert('Поездка отмечена как незавершенная');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при отклонении поездки: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при отклонении поездки:', error);
        alert('Ошибка при отклонении поездки');
    }
}

// Функция для пропуска подтверждения поездки
async function skipTrip(rentalId) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите пропустить подтверждение этой поездки?')) {
        return;
    }
    
    alert('Подтверждение поездки пропущено');
    // Просто перезагружаем таблицу, ничего не изменяя
    loadTripCompletions();
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadTripCompletions);