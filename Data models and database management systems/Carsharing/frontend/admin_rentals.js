// Функция для загрузки и отображения данных таблицы Rentals
async function loadRentals() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем все аренды (с большим лимитом)
        const response = await fetch('/rentals/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const rentals = await response.json();
            displayRentals(rentals);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Rentals: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Rentals:', error);
        alert('Ошибка при загрузке Rentals');
    }
}

// Функция для отображения данных в таблице Rentals
function displayRentals(rentals) {
    const tableBody = document.getElementById('rentals-table-body');
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
        `;
        tableBody.appendChild(row);
    });
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadRentals);