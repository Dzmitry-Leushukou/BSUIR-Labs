// Функция для загрузки и отображения данных таблицы Driver licenses
async function loadDriverLicenses() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем все водительские лицензии (с большим лимитом)
        const response = await fetch('/driver_licenses/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const driverLicenses = await response.json();
            displayDriverLicenses(driverLicenses);
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                // Проверяем, является ли ответ JSON
                const contentType = response.headers.get('content-type');
                if (contentType && contentType.includes('application/json')) {
                    const errorData = await response.json();
                    errorDetail = errorData.detail || errorData.message || JSON.stringify(errorData);
                } else {
                    // Если ответ не JSON, получаем текст
                    errorDetail = await response.text();
                }
            } catch (e) {
                // Если не удалось распарсить JSON или получить текст, используем код статуса
                errorDetail = `HTTP Error ${response.status}: ${response.statusText}`;
            }
            alert(`Ошибка при загрузке Driver licenses: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Driver licenses:', error);
        alert('Ошибка при загрузке Driver licenses');
    }
}

// Функция для отображения данных в таблице Driver licenses
async function displayDriverLicenses(driverLicenses) {
    const tableBody = document.getElementById('driver-licenses-table-body');
    tableBody.innerHTML = '';
    
    for (const license of driverLicenses) {
        // Загружаем информацию о фотографии
        let photoUrl = '';
        if (license.document_photo_id) {
            try {
                const userId = localStorage.getItem('user_id');
                const photoResponse = await fetch(`/photos/${license.document_photo_id}`, {
                    method: 'GET',
                    headers: {
                        'X-User-ID': userId,
                        'Content-Type': 'application/json'
                    }
                });
                
                if (photoResponse.ok) {
                    const photo = await photoResponse.json();
                    photoUrl = photo.url;
                }
            } catch (error) {
                console.error('Ошибка при загрузке информации о фотографии:', error);
            }
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${license.driver_id}</td>
            <td>${license.license_number}</td>
            <td>${license.issued_by}</td>
            <td>${new Date(license.expiration_date).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
            <td>${license.document_photo_id}</td>
            <td>
                ${photoUrl ? `<img src="${photoUrl}" alt="Document Photo" style="max-width: 100px; max-height: 100px; cursor: pointer;" onclick="showPhotoModal('${photoUrl}', 'Фото документа')">` : 'Нет фото'}
            </td>
            <td class="status-${license.status}">${license.status}</td>
            <td>
                <button class="btn action-btn approve-btn" onclick="updateLicenseStatus(${license.driver_id}, 'approved')">Подтвердить</button>
                <button class="btn action-btn reject-btn" onclick="updateLicenseStatus(${license.driver_id}, 'rejected')">Отклонить</button>
            </td>
        `;
        tableBody.appendChild(row);
    }
}

// Функция для обновления статуса водительской лицензии
async function updateLicenseStatus(licenseId, status) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm(`Вы уверены, что хотите изменить статус лицензии на "${status}"?`)) {
        return;
    }
    
    try {
        const response = await fetch(`/driver_licenses/${licenseId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: status })
        });
        
        if (response.ok) {
            alert(`Статус лицензии успешно обновлен на "${status}"`);
            // Перезагружаем таблицу
            loadDriverLicenses();
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                // Проверяем, является ли ответ JSON
                const contentType = response.headers.get('content-type');
                if (contentType && contentType.includes('application/json')) {
                    const errorData = await response.json();
                    errorDetail = errorData.detail || errorData.message || JSON.stringify(errorData);
                } else {
                    // Если ответ не JSON, получаем текст
                    errorDetail = await response.text();
                }
            } catch (e) {
                // Если не удалось распарсить JSON или получить текст, используем код статуса
                errorDetail = `HTTP Error ${response.status}: ${response.statusText}`;
            }
            alert(`Ошибка при обновлении статуса лицензии: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении статуса лицензии:', error);
        alert('Ошибка при обновлении статуса лицензии');
    }
}

// Функция для фильтрации данных таблицы Driver licenses
function filterDriverLicenses() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#driver-licenses-table-body tr');
    
    rows.forEach(row => {
        let shouldShow = true;
        
        // Проверяем каждую ячейку в строке (пропускаем колонку с фото и действиями)
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Для колонки с фото и действиями пропускаем фильтрацию
                // В таблице водительских лицензий колонки с фото и действиями находятся в позициях 5 и 7 (0-индексированные)
                if (i === 5 || i === 7) continue; // Пропускаем колонки с фотографией и действиями
                
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
    loadDriverLicenses();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterDriverLicenses);
    });
});