// Функция для загрузки и отображения данных таблицы Cars
async function loadCars() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/cars/', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const cars = await response.json();
            displayCars(cars);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Cars: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Cars:', error);
        alert('Ошибка при загрузке Cars');
    }
}

// Функция для отображения данных в таблице Cars
function displayCars(cars) {
    const tableBody = document.getElementById('cars-table-body');
    tableBody.innerHTML = '';
    
    cars.forEach(car => {
        // Форматируем координаты из геометрии
        let position = '';
        if (car.position) {
            // Пример: "POINT(27.5615 53.9041)" -> "53.9041, 27.5615"
            const match = car.position.match(/POINT\(([-+]?\d*\.\d+|\d+) ([-+]?\d*\.\d+|\d+)\)/);
            if (match) {
                position = `${match[2]}, ${match[1]}`; // широта, долгота
            }
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${car.id}</td>
            <td>${car.vin}</td>
            <td>${car.plate_number}</td>
            <td>${car.model}</td>
            <td>${car.status}</td>
            <td>${position}</td>
            <td>${new Date(car.updated_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadCars);