// Глобальные переменные для пагинации
let currentDriverLicensePage = 0;
const driverLicensesPerPage = 10;

// Функция для загрузки и отображения данных таблицы Driver licenses с пагинацией
async function loadDriverLicenses(page = 0) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentDriverLicensePage = validPageNum;
    const offset = validPageNum * driverLicensesPerPage;
    
    try {
        // Загружаем водительские лицензии с пагинацией
        const response = await fetch(`/driver_licenses/?offset=${offset}&limit=${driverLicensesPerPage}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const driverLicenses = await response.json();
            displayDriverLicenses(driverLicenses);
            setupDriverLicensePagination(page);
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
                // Проверяем, является ли ответ JSON
                const contentType = response.headers.get('content-type');
                if (contentType && contentType.includes('application/json')) {
                    const errorData = await response.json();
                    if (typeof errorData.detail === 'string') {
                        errorDetail = errorData.detail;
                    } else if (typeof errorData.message === 'string') {
                        errorDetail = errorData.message;
                    } else if (typeof errorData === 'object' && errorData !== null) {
                        // Проверяем наличие других полей с сообщениями об ошибке
                        if (errorData.error) {
                            errorDetail = errorData.error;
                        } else if (errorData.msg) {
                            errorDetail = errorData.msg;
                        } else {
                            // Если объект сложный, пытаемся получить читаемое сообщение
                            // Проверяем наличие других полей с сообщениями об ошибке
                            if (errorData.error) {
                                errorDetail = errorData.error;
                            } else if (errorData.msg) {
                                errorDetail = errorData.msg;
                            } else {
                                errorDetail = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                            }
                        }
                    } else {
                        // Проверяем наличие других полей с сообщениями об ошибке
                        if (errorData && typeof errorData === 'object') {
                            if (errorData.error) {
                                errorDetail = errorData.error;
                            } else if (errorData.msg) {
                                errorDetail = errorData.msg;
                            } else {
                                errorDetail = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                            }
                        } else {
                            errorDetail = String(errorData);
                        }
                    }
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

// Функция для настройки пагинации водительских лицензий
function setupDriverLicensePagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество водительских лицензий для определения количества страниц
    getDriverLicensesCount().then(totalCount => {
        const totalPages = Math.ceil(totalCount / driverLicensesPerPage);
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('driver-licenses-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'driver-licenses-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#driver-licenses-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'driver-licenses-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#driver-licenses-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#driver-licenses-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#driver-licenses-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadDriverLicenses(${safePageNum - 1})">Предыдущая</button>`;
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
                paginationHTML += `<button class="pagination-btn" onclick="loadDriverLicenses(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadDriverLicenses(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    });
}

// Функция для получения общего количества водительских лицензий
async function getDriverLicensesCount() {
    try {
        const response = await fetch('/driver_licenses/count', {
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
        console.error('Ошибка при получении количества водительских лицензий:', error);
        return 0;
    }
}

// Функция для отображения данных в таблице Driver licenses
async function displayDriverLicenses(driverLicenses) {
    const tableBody = document.getElementById('driver-licenses-table-body');
    tableBody.innerHTML = '';
    
    for (const license of driverLicenses) {
        // Формируем URL для изображений напрямую
        let photoUrl = license.document_photo_id ? `/photos/file/${license.document_photo_id}` : '';
        let photoBackUrl = license.document_photo_back_id ? `/photos/file/${license.document_photo_back_id}` : '';
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${license.driver_id}</td>
            <td>${license.license_number}</td>
            <td>${license.issued_by}</td>
            <td>${new Date(license.expiration_date).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${license.document_photo_id}</td>
            <td>
                ${photoUrl ? `<img src="${photoUrl}" alt="Document Photo" style="max-width: 100px; max-height: 100px; cursor: pointer; border: 2px solid #ddd; border-radius: 4px;" onclick="showPhotoModal('${photoUrl}', 'Фото документа (лицевая сторона)')" title="Кликните для просмотра в полном размере">` : '-'}
            </td>
            <td>${license.document_photo_back_id}</td>
            <td>
                ${photoBackUrl ? `<img src="${photoBackUrl}" alt="Document Back Photo" style="max-width: 100px; max-height: 100px; cursor: pointer; border: 2px solid #ddd; border-radius: 4px;" onclick="showPhotoModal('${photoBackUrl}', 'Фото документа (обратная сторона)')" title="Кликните для просмотра в полном размере">` : '-'}
            </td>
            <td class="status-${license.status}">${license.status === 'pending' ? 'Ожидает проверки' : license.status === 'approved' ? 'Подтверждено' : license.status === 'rejected' ? 'Отклонено' : license.status}</td>
            <td>
                ${license.status === 'pending' ? `
                    <button class="btn action-btn approve-btn" onclick="updateLicenseStatus(${license.driver_id}, 'approved')">Подтвердить</button>
                    <button class="btn action-btn reject-btn" onclick="updateLicenseStatus(${license.driver_id}, 'rejected')">Отклонить</button>
                ` : license.status === 'approved' ? `
                    <button class="btn action-btn reject-btn" onclick="updateLicenseStatus(${license.driver_id}, 'rejected')">Отклонить</button>
                ` : license.status === 'rejected' ? `
                    <button class="btn action-btn approve-btn" onclick="updateLicenseStatus(${license.driver_id}, 'approved')">Подтвердить</button>
                ` : ''}
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
                    if (typeof errorData.detail === 'string') {
                        errorDetail = errorData.detail;
                    } else if (typeof errorData.message === 'string') {
                        errorDetail = errorData.message;
                    } else if (typeof errorData === 'object' && errorData !== null) {
                        // Проверяем наличие других полей с сообщениями об ошибке
                        if (errorData.error) {
                            errorDetail = errorData.error;
                        } else if (errorData.msg) {
                            errorDetail = errorData.msg;
                        } else {
                            // Если объект сложный, пытаемся получить читаемое сообщение
                            // Проверяем наличие других полей с сообщениями об ошибке
                            if (errorData.error) {
                                errorDetail = errorData.error;
                            } else if (errorData.msg) {
                                errorDetail = errorData.msg;
                            } else {
                                errorDetail = JSON.stringify(errorData);
                            }
                        }
                    } else {
                        // Проверяем наличие других полей с сообщениями об ошибке
                        if (errorData && typeof errorData === 'object') {
                            if (errorData.error) {
                                errorDetail = errorData.error;
                            } else if (errorData.msg) {
                                errorDetail = errorData.msg;
                            } else {
                                errorDetail = JSON.stringify(errorData);
                            }
                        } else {
                            errorDetail = String(errorData);
                        }
                    }
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
        
        // Проверяем каждую ячейку в строке (пропускаем колонки с фото и действиями)
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Для колонок с фото и действиями пропускаем фильтрацию
                // В таблице водительских лицензий колонки с фото и действиями находятся в позициях 5, 7 и 9 (0-индексированные)
                if (i === 5 || i === 7 || i === 9) continue; // Пропускаем колонки с фотографиями и действиями
                
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
    loadDriverLicenses(0);  // Загружаем первую страницу
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterDriverLicenses);
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