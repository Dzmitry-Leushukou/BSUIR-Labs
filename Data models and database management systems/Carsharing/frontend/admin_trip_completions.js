// Глобальные переменные для пагинации
let currentTripCompletionPage = 0;
const tripCompletionsPerPage = 10;

// Функция для загрузки и отображения данных таблицы Trip completions с пагинацией
async function loadTripCompletions(page = 0) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentTripCompletionPage = validPageNum;
    const offset = validPageNum * tripCompletionsPerPage;
    
    try {
        const response = await fetch(`/trip-completions/?offset=${offset}&limit=${tripCompletionsPerPage}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const tripCompletions = await response.json();
            displayTripCompletionsData(tripCompletions);
            setupTripCompletionPagination(page);
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
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
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                try {
                    errorDetail = await response.text() || 'Неизвестная ошибка';
                } catch (textError) {
                    errorDetail = 'Неизвестная ошибка';
                }
            }
            alert(`Ошибка при загрузке поездок: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке поездок:', error);
        alert('Ошибка при загрузке поездок');
    }
}

// Функция для настройки пагинации завершений поездок
function setupTripCompletionPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество завершений поездок для определения количества страниц
    getTripCompletionsCount().then(totalCount => {
        const totalPages = Math.ceil(totalCount / tripCompletionsPerPage);
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('trip-completions-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'trip-completions-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#trip-completions-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'trip-completions-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#trip-completions-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#trip-completions-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#trip-completions-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadTripCompletions(${safePageNum - 1})">Предыдущая</button>`;
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
                paginationHTML += `<button class="pagination-btn" onclick="loadTripCompletions(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadTripCompletions(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    });
}

// Функция для получения общего количества завершений поездок
async function getTripCompletionsCount() {
    try {
        const userId = localStorage.getItem('user_id');
        if (!userId) {
            throw new Error('Пользователь не авторизован');
        }
        
        const response = await fetch('/trip-completions/count', {
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
            console.error('Ошибка сервера при получении количества завершений поездок:', errorData);
            return 0;
        }
    } catch (error) {
        console.error('Ошибка при получении количества завершений поездок:', error);
        return 0;
    }
}

// Функция для отображения данных в таблице Trip completions
async function displayTripCompletionsData(tripCompletions) {
    const tableBody = document.getElementById('trip-completions-table-body');
    tableBody.innerHTML = '';
    
    for (const completion of tripCompletions) {
        // First get the associated rental to get user_id, car_id, etc.
        let rental = null;
        try {
            const userId = localStorage.getItem('user_id');
            const rentalResponse = await fetch(`/rentals/${completion.rental_id}`, {
                method: 'GET',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                }
            });
            
            if (rentalResponse.ok) {
                rental = await rentalResponse.json();
            }
        } catch (error) {
            console.error('Ошибка при загрузке данных аренды:', error);
        }
        
        // Загружаем фотографии завершения поездки
        let photosHtml = 'Нет фото';
        try {
            const userId = localStorage.getItem('user_id');
            
            // Check if completion has multiple photo IDs
            if (completion.completion_photo_ids && Array.isArray(completion.completion_photo_ids) && completion.completion_photo_ids.length > 0) {
                photosHtml = '';
                for (const photoId of completion.completion_photo_ids) {
                    const photosResponse = await fetch(`/photos/${photoId}`, {
                        method: 'GET',
                        headers: {
                            'X-User-ID': userId,
                            'Content-Type': 'application/json'
                        }
                    });
                    
                    if (photosResponse.ok) {
                        const photo = await photosResponse.json();
                        if (photo) {
                            // Using the correct endpoint to download the photo
                            const photoUrl = `/photos/file/${photo.id}`; // Correct endpoint based on the backend implementation
                            photosHtml += `<img src="${photoUrl}" alt="Completion Photo" style="max-width: 50px; max-height: 50px; margin: 2px; cursor: pointer; border: 2px solid #ddd; border-radius: 4px;" onclick="showPhotoModal('${photoUrl}', 'Фото завершения поездки')" title="Кликните для просмотра в полном размере">`;
                        }
                    }
                }
            }
        } catch (error) {
            console.error('Ошибка при загрузке фотографии завершения поездки:', error);
        }
        
        // Determine status text based on admin approval
        let statusText = 'На рассмотрении';
        if (completion.admin_approved === true) {
            statusText = 'Подтверждено';
        } else if (completion.admin_approved === false) {
            statusText = 'Обнаружены повреждения';
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${completion.id}</td>
            <td>${rental ? rental.user_id : 'N/A'}</td>
            <td>${rental ? rental.car_id : 'N/A'}</td>
            <td>${rental ? new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' }) : 'N/A'}</td>
            <td>${rental ? (rental.ended_at ? new Date(rental.ended_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' }) : '') : ''}</td>
            <td>${rental ? rental.price + ' BYN' : 'N/A'}</td>
            <td>${photosHtml}</td>
            <td class="status-${completion.admin_approved === null ? 'pending' : completion.admin_approved ? 'approved' : 'rejected'}">
                ${completion.admin_approved === null ? 'На рассмотрении' : completion.admin_approved ? 'Подтверждено' : 'Обнаружены повреждения'}
                ${completion.admin_comment ? '<br><small><strong>Комментарий администратора:</strong> ' + completion.admin_comment + '</small>' : ''}
            </td>
            <td>
                ${completion.admin_approved === null ? `
                    <button class="btn action-btn approve-btn" onclick="confirmTrip(${completion.id})">Повреждений нет</button>
                    <button class="btn action-btn damage-btn" onclick="reportDamage(${completion.id})">Обнаружены повреждения</button>
                ` : completion.admin_approved === true ? `
                    <!-- Кнопки скрыты, так как подтверждено, что повреждений нет -->
                ` : completion.admin_approved === false ? `
                    <!-- Кнопки скрыты, так как уже отмечены повреждения -->
                ` : ''}
            </td>
        `;
        tableBody.appendChild(row);
    }
}

// Функция для подтверждения завершения поездки (без повреждений)
async function confirmTrip(completionId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите подтвердить завершение поездки (без повреждений)?')) {
        return;
    }
    
    try {
        const response = await fetch(`/trip-completions/${completionId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                admin_approved: true,
                admin_comment: null
            })
        });
        
        if (response.ok) {
            alert('Поездка успешно подтверждена');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
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
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                try {
                    errorDetail = await response.text() || 'Неизвестная ошибка';
                } catch (textError) {
                    errorDetail = 'Неизвестная ошибка';
                }
            }
            alert(`Ошибка при подтверждении поездки: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при подтверждении поездки:', error);
        alert('Ошибка при подтверждении поездки');
    }
}

// Функция для сообщения о повреждениях
async function reportDamage(completionId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const damageDescription = prompt('Введите описание повреждений:');
    if (damageDescription === null) {
        // User cancelled
        return;
    }
    
    if (damageDescription.trim() === '') {
        alert('Пожалуйста, введите описание повреждений');
        return;
    }
    
    try {
        const response = await fetch(`/trip-completions/${completionId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                admin_approved: false,  // Mark as not approved due to damage
                admin_comment: damageDescription
            })
        });
        
        if (response.ok) {
            alert('Повреждения успешно зарегистрированы. Создан запрос на обслуживание.');
            // Перезагружаем таблицу
            loadTripCompletions();
        } else {
            let errorDetail = 'Неизвестная ошибка';
            try {
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
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                try {
                    errorDetail = await response.text() || 'Неизвестная ошибка';
                } catch (textError) {
                    errorDetail = 'Неизвестная ошибка';
                }
            }
            alert(`Ошибка при регистрации повреждений: ${errorDetail}`);
        }
    } catch (error) {
        console.error('Ошибка при регистрации повреждений:', error);
        alert('Ошибка при регистрации повреждений');
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
    loadTripCompletions(0);  // Загружаем первую страницу
    
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