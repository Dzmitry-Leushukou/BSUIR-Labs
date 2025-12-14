// Функция для загрузки и отображения данных таблицы Trip completions
async function loadTripCompletions() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/trip-completions/', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const tripCompletions = await response.json();
            // Trip completions are already sorted by backend (oldest to newest, confirmed at end), so just display them
            displayTripCompletionsData(tripCompletions);
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
            const photosResponse = await fetch(`/photos/${completion.completion_photo_id}`, {
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
                    photosHtml = `<img src="${photoUrl}" alt="Completion Photo" style="max-width: 50px; max-height: 50px; margin: 2px; cursor: pointer; border: 2px solid #ddd; border-radius: 4px;" onclick="showPhotoModal('${photoUrl}', 'Фото завершения поездки')" title="Кликните для просмотра в полном размере">`;
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