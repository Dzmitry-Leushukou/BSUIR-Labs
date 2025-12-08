// Функция для загрузки и отображения данных таблицы Trip completions
async function loadTripCompletions() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/rentals/', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const rentals = await response.json();
            // Отображаем поездки, которые требуют подтверждения администратором (у которых есть время окончания, но статус еще не 'completed')
            const tripCompletions = rentals.filter(rental => rental.status === 'active' && rental.ended_at !== null);
            displayTripCompletions(tripCompletions);
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                const errorData = await response.json();
                errorDetail = errorData.detail || errorData.message || JSON.stringify(errorData);
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                errorDetail = await response.text() || 'Неизвестная ошибка';
            }
            alert(`Ошибка при загрузке поездок: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке поездок:', error);
        alert('Ошибка при загрузке поездок');
    }
}

// Функция для отображения данных в таблице Trip completions
async function displayTripCompletions(rentals) {
    const tableBody = document.getElementById('trip-completions-table-body');
    tableBody.innerHTML = '';
    
    for (const rental of rentals) {
        // Загружаем фотографии автомобиля
        let photosHtml = 'Нет фото';
        try {
            const userId = localStorage.getItem('user_id');
            const photosResponse = await fetch(`/photos/?object_type=car&car_id=${rental.car_id}`, {
                method: 'GET',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                }
            });
            
            if (photosResponse.ok) {
                const photos = await photosResponse.json();
                if (photos && photos.length > 0) {
                    photosHtml = photos.map(photo =>
                        `<img src="${photo.url}" alt="Car Photo" style="max-width: 50px; max-height: 50px; margin: 2px; cursor: pointer;" onclick="showPhotoModal('${photo.url}', 'Фото автомобиля')">`
                    ).join('');
                }
            }
        } catch (error) {
            console.error('Ошибка при загрузке фотографий автомобиля:', error);
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${rental.id}</td>
            <td>${rental.user_id}</td>
            <td>${rental.car_id}</td>
            <td>${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
            <td>${rental.ended_at ? new Date(rental.ended_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' }) : ''}</td>
            <td>${rental.price} BYN</td>
            <td>${photosHtml}</td>
            <td class="status-${rental.status}">${rental.status}</td>
            <td>
                <button class="btn action-btn approve-btn" onclick="confirmTrip(${rental.id})">Подтвердить</button>
                <button class="btn action-btn reject-btn" onclick="rejectTrip(${rental.id})">Отклонить</button>
            </td>
        `;
        tableBody.appendChild(row);
    }
}

// Функция для подтверждения завершения поездки
async function confirmTrip(rentalId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
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
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: 'completed' })
        });
        
        if (response.ok) {
            alert('Поездка успешно подтверждена');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                const errorData = await response.json();
                errorDetail = errorData.detail || errorData.message || JSON.stringify(errorData);
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                errorDetail = await response.text() || 'Неизвестная ошибка';
            }
            alert(`Ошибка при подтверждении поездки: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при подтверждении поездки:', error);
        alert('Ошибка при подтверждении поездки');
    }
}

// Функция для отклонения завершения поездки
async function rejectTrip(rentalId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
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
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: 'cancelled' }) // Устанавливаем статус отмененной поездки
        });
        
        if (response.ok) {
            alert('Поездка отмечена как отмененная');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                const errorData = await response.json();
                errorDetail = errorData.detail || errorData.message || JSON.stringify(errorData);
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                errorDetail = await response.text() || 'Неизвестная ошибка';
            }
            alert(`Ошибка при отклонении поездки: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при отклонении поездки:', error);
        alert('Ошибка при отклонении поездки');
    }
}


// Функция для отображения модального окна с фотографией
function showPhotoModal(photoUrl, title) {
    // Удаляем предыдущее модальное окно, если оно существует
    const existingModal = document.getElementById('photo-modal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // Создаем модальное окно
    const modal = document.createElement('div');
    modal.id = 'photo-modal';
    modal.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background-color: rgba(0,0,0.8);
        display: flex;
        justify-content: center;
        align-items: center;
        z-index: 10000;
        cursor: pointer;
    `;
    
    modal.innerHTML = `
        <div style="position: relative; max-width: 90%; max-height: 90%;">
            <img src="${photoUrl}" alt="${title}" style="max-width: 100%; max-height: 100%; display: block;">
            <span style="
                position: absolute;
                top: -30px;
                right: 0;
                color: white;
                font-size: 30px;
                font-weight: bold;
                cursor: pointer;
                background: #333;
                border-radius: 50%;
                width: 30px;
                height: 30px;
                display: flex;
                align-items: center;
                justify-content: center;
            " onclick="closePhotoModal()">×</span>
        </div>
    `;
    
    document.body.appendChild(modal);
}

// Функция для закрытия модального окна с фотографией
function closePhotoModal() {
    const modal = document.getElementById('photo-modal');
    if (modal) {
        modal.remove();
    }
}

// Функция для фильтрации данных таблицы Trip completions
function filterTripCompletions() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#trip-completions-table-body tr');
    
    rows.forEach(row => {
        let shouldShow = true;
        
        // Проверяем каждую ячейку в строке (пропускаем колонку с фото и действиями)
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Для колонки с фото и действиями пропускаем фильтрацию
                if (i === 6 || i === 8) continue; // Пропускаем колонки с фотографиями и действиями
                
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
    loadTripCompletions();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterTripCompletions);
    });
});

// Добавляем обработчик клика на документ для закрытия модального окна
document.addEventListener('click', function(event) {
    const modal = document.getElementById('photo-modal');
    if (modal && event.target === modal) {
        closePhotoModal();
    }
});