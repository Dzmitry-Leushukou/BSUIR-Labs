 е// Функция для возврата на предыдущую страницу
function goBack() {
    window.history.back();
}

// Инициализация карты
let profileMap;
let profileMarker;

// Загрузка информации о пользователе при загрузке страницы профиля
document.addEventListener('DOMContentLoaded', async () => {
    await loadProfileInfo();
    
    // Не инициализируем карту на странице профиля, так как она там не нужна
    // initProfileMap();
});

// Функция для инициализации карты на странице профиля
function initProfileMap() {
    // Получаем текущее местоположение пользователя
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
            (position) => {
                const { latitude, longitude } = position.coords;
                
                // Инициализация карты с текущим местоположением
                initializeMap(latitude, longitude);
                
                // Добавление маркера на карту
                addProfileMarker(latitude, longitude);
                
                // Показать машины на карте
                showProfileCarsOnMap();
            },
            (error) => {
                console.error('Ошибка получения местоположения:', error);
                // Используем Минск как fallback
                initializeMap(53.904133, 27.557541);
                
                // Показать машины на карте
                showProfileCarsOnMap();
            }
        );
    } else {
        console.error('Геолокация не поддерживается браузером');
        // Используем Минск как fallback
        initializeMap(53.904133, 27.557541);
        
        // Показать машины на карте
        showProfileCarsOnMap();
    }
}

// Инициализация карты с Leaflet
function initializeMap(lat, lng) {
    if (profileMap) {
        profileMap.remove();
    }
    
    profileMap = L.map('map').setView([lat, lng], 13);
    
    // Добавление слоя карты OpenStreetMap
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(profileMap);
}

// Добавление маркера на карту
function addProfileMarker(lat, lng) {
    if (profileMarker) {
        profileMap.removeLayer(profileMarker);
    }
    
    // Создаем маркер с пользовательской иконкой для местоположения пользователя
    const userIcon = L.divIcon({
        className: 'user-location-marker',
        html: '<div style="background-color: #007bff; color: white; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; border: 2px solid white; box-shadow: 0 0 5px rgba(0,0,0,0.5);">👤</div>',
        iconSize: [24, 24],
        iconAnchor: [12, 12]
    });
    
    profileMarker = L.marker([lat, lng], {icon: userIcon}).addTo(profileMap);
    profileMarker.bindPopup('Ваше текущее местоположение').openPopup();
}

// Показать все машины на карте
async function showProfileCarsOnMap() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        console.error('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/cars/all/positions', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const cars = await response.json();
            
            // Добавить маркеры для каждой машины
            cars.forEach(car => {
                if (car.latitude && car.longitude) {
                    // Создаем иконку для маркера машины
                    const carIcon = L.divIcon({
                        className: 'car-marker',
                        html: '<div style="background-color: #28a745; color: white; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; border: 2px solid white; box-shadow: 0 0 5px rgba(0,0,0,0.5);">🚗</div>',
                        iconSize: [24, 24],
                        iconAnchor: [12, 12]
                    });
                    
                    const carMarker = L.marker([car.latitude, car.longitude], {icon: carIcon}).addTo(profileMap);
                    carMarker.bindPopup(`
                        <b>Машина: ${car.model}</b><br>
                        Номер: ${car.plate_number}<br>
                        Статус: ${car.status}
                    `);
                }
            });
        } else {
            console.error('Ошибка при получении позиций машин:', response.status);
        }
    } catch (error) {
        console.error('Ошибка при запросе позиций машин:', error);
    }
}

// Убираем функции, связанные с отображением активной поездки на странице профиля
// Функция checkActiveTrip и связанные с ней функции не должны выполняться на странице профиля,
// так как активная поездка отображается на главной странице

// Функция для загрузки информации о пользователе
async function loadProfileInfo() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        // Если пользователь не авторизован, перенаправляем на главную страницу
        window.location.href = '/';
        return;
    }
    
    try {
        const response = await fetch('/users/profile', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
                    const userData = await response.json();
                    
                    // Проверяем статус пользователя
                    if (userData.status === 'banned') {
                        // Показываем сообщение о бане
                        const profileContainer = document.querySelector('.profile-container');
                        const banMessage = document.createElement('div');
                        banMessage.className = 'ban-message';
                        banMessage.innerHTML = '<h3 style="color: red; text-align: center;">Ваш аккаунт заблокирован</h3>';
                        profileContainer.insertBefore(banMessage, profileContainer.firstChild);
                    }
                    
                    // Заполняем информацию о пользователе
                    document.getElementById('profile-name').textContent = userData.status === 'banned' ? '' : userData.name;
                    document.getElementById('profile-surname').textContent = userData.status === 'banned' ? '' : userData.surname;
                    document.getElementById('profile-email').textContent = userData.status === 'banned' ? '' : userData.email;
                    document.getElementById('profile-cashback').textContent = userData.status === 'banned' ? '' : `${userData.cashback} BYN`;
                    
                    // Получаем информацию о роли пользователя из базы данных
                    const roleResponse = await fetch(`/roles/${userData.role_id}`);
                    if (roleResponse.ok) {
                        const roleData = await roleResponse.json();
                        document.getElementById('profile-role').textContent = userData.status === 'banned' ? '' : roleData.name;
                    } else {
                        // В случае ошибки используем резервный вариант
                        let roleName;
                        if (userData.role_id === 1) {
                            roleName = 'Администратор';
                        } else if (userData.role_id === 2) {
                            roleName = 'Пользователь';
                        } else {
                            // Обработка случая, когда role_id неизвестен (например, 0 или другое значение)
                            const errorData = await roleResponse.json();
                            console.error(`Ошибка получения информации о роли: ${errorData.detail || 'Неизвестная ошибка'}`);
                            roleName = `Неизвестная роль (ID: ${userData.role_id})`;
                        }
                        document.getElementById('profile-role').textContent = userData.status === 'banned' ? '' : roleName;
                    }
                    
                    // Сохраняем user_id, если он был возвращен с сервера
                                if (userData.id) {
                                    localStorage.setItem('user_id', userData.id);
                                }
                                
                    // Убираем проверку активной аренды на странице профиля, так как завершение аренды происходит на главной странице
                    // await checkActiveRental();
                    
                    // Обновляем видимость кнопки админ панели в зависимости от роли пользователя
                    const adminPanelButton = document.getElementById('admin-panel-button');
                    if (adminPanelButton) {
                        if (userData.status === 'banned' || userData.role_id !== 1) { // admin role ID is 1
                            adminPanelButton.style.display = 'none';
                        } else {
                            adminPanelButton.style.display = 'block';
                        }
                    }
                    
                    // Загружаем статус водительских прав
                                await loadDriverLicenseStatus(userId);
                                
                                // Начинаем отслеживание изменений статуса водительских прав
                                startLicenseStatusPolling();
                            } else {
                    // Если user_id недействителен, удаляем его и перенаправляем на главную страницу
                                localStorage.removeItem('user_id');
                                window.location.href = '/';
                                
                    // Также скрываем кнопку админ панели если пользователь не авторизован
                    const adminPanelButton = document.getElementById('admin-panel-button');
                    if (adminPanelButton) {
                        adminPanelButton.style.display = 'none';
                    }
                }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
        // При ошибке перенаправляем на главную страницу
        window.location.href = '/';
        
        // Также скрываем кнопку админ панели в случае ошибки
        const adminPanelButton = document.getElementById('admin-panel-button');
        if (adminPanelButton) {
            adminPanelButton.style.display = 'none';
        }
    }
    
}

// Функция для загрузки статуса водительских прав
async function loadDriverLicenseStatus(userId) {
    // Проверяем статус пользователя
    const response = await fetch('/users/profile', {
        method: 'GET',
        headers: {
            'X-User-ID': userId,
            'Content-Type': 'application/json'
        }
    });
    
    if (response.ok) {
        const userData = await response.json();
        if (userData.status === 'banned') {
            // Если пользователь забанен, не загружаем статус водительских прав
            document.getElementById('profile-license-status').textContent = '';
            document.getElementById('profile-license-status').className = 'status-not-loaded';
            return 'banned';
        }
    }
    
    try {
        // Сначала получаем все водительские права пользователя
        const response = await fetch('/driver_licenses/', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const licenses = await response.json();
            // Находим права, связанные с текущим пользователем (предполагаем, что driver_id в таблице прав соответствует user_id)
            const userLicense = licenses.find(license => license.driver_id == userId);
            
            if (userLicense) {
                // Обновляем отображение статуса водительских прав с цветовой индикацией
                const licenseStatusElement = document.getElementById('profile-license-status');
                licenseStatusElement.textContent = userLicense.status === 'pending' ? 'Ожидает проверки' : userLicense.status === 'approved' ? 'Подтверждено' : userLicense.status === 'rejected' ? 'Отклонено' : userLicense.status;
                licenseStatusElement.className = `status-${userLicense.status}`;
                
                // Возвращаем статус для возможного использования в других функциях
                return userLicense.status;
            } else {
                // Если права не найдены, отображаем "Не отправлены на проверку"
                document.getElementById('profile-license-status').textContent = 'Не отправлены на проверку';
                document.getElementById('profile-license-status').className = 'status-not-loaded';
                return 'not_submitted';
            }
        } else {
            // Если произошла ошибка, отображаем "Не загружены"
            document.getElementById('profile-license-status').textContent = 'Не загружены';
            document.getElementById('profile-license-status').className = 'status-not-loaded';
            return 'error';
        }
    } catch (error) {
        console.error('Ошибка при загрузке статуса водительских прав:', error);
        document.getElementById('profile-license-status').textContent = 'Не загружены';
        document.getElementById('profile-license-status').className = 'status-not-loaded';
        return 'error';
    }
}

// Функция для периодического обновления статуса водительских прав без перезагрузки страницы
function startLicenseStatusPolling() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        console.error('Пользователь не авторизован');
        return;
    }
    
    let pollingInterval;
    
    // Функция для проверки статуса
    const checkStatus = async () => {
        const currentStatusElement = document.getElementById('profile-license-status');
        if (!currentStatusElement) return;
        
        // Получаем текущий статус из элемента (для сравнения)
        const currentDisplayStatus = currentStatusElement.textContent;
        
        // Загружаем актуальный статус
        const newStatus = await loadDriverLicenseStatus(userId);
        
        // Если статус изменился, можно выполнить дополнительные действия
        if (newStatus && newStatus !== 'error' && newStatus !== 'not_submitted') {
            // Получаем текстовое представление статуса для сравнения
            const statusText = newStatus === 'pending' ? 'Ожидает проверки' :
                              newStatus === 'approved' ? 'Подтверждено' :
                              newStatus === 'rejected' ? 'Отклонено' : newStatus;
            
            if (currentDisplayStatus !== statusText) {
                console.log('Статус водительских прав обновлён:', statusText);
                // Можно добавить визуальное уведомление об изменении статуса
                // Например, кратковременно выделить элемент
                currentStatusElement.style.fontWeight = 'bold';
                setTimeout(() => {
                    currentStatusElement.style.fontWeight = '';
                }, 2000);
            }
        }
    };
    
    // Проверяем статус каждые 5 секунд когда вкладка активна
    pollingInterval = setInterval(checkStatus, 5000);
    
    // Оптимизация: останавливаем опрос когда вкладка неактивна и возобновляем когда активна
    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'hidden') {
            // Вкладка неактивна - останавливаем опрос
            clearInterval(pollingInterval);
        } else {
            // Вкладка снова активна - возобновляем опрос
            pollingInterval = setInterval(checkStatus, 5000);
        }
    });
}

// Обработчик для кнопки "Редактировать профиль"
document.querySelector('.edit-profile-btn').addEventListener('click', () => {
    openEditProfileModal();
});

// Обработчик для кнопки "Сменить пароль"
document.querySelector('.change-password-btn').addEventListener('click', () => {
    openChangePasswordModal();
});

// Функция открытия модального окна смены пароля
function openChangePasswordModal() {
    // Очищаем форму
    document.getElementById('change-password-form').reset();
    
    // Показываем модальное окно
    document.getElementById('change-password-modal').style.display = 'block';
}

// Закрытие модального окна смены пароля при клике на крестик
document.querySelector('.close-change-password').addEventListener('click', () => {
    document.getElementById('change-password-modal').style.display = 'none';
});

// Закрытие модального окна смены пароля при клике на кнопку "Отмена"
document.querySelector('.cancel-change-password').addEventListener('click', () => {
    document.getElementById('change-password-modal').style.display = 'none';
});

// Закрытие модального окна смены пароля при клике вне его области
window.addEventListener('click', (event) => {
    const modal = document.getElementById('change-password-modal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
});

// Обработчик отправки формы смены пароля
document.getElementById('change-password-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const currentPassword = document.getElementById('current-password').value;
    const newPassword = document.getElementById('new-password').value;
    const confirmPassword = document.getElementById('confirm-new-password').value;
    
    // Проверяем, совпадают ли новые пароли
    if (newPassword !== confirmPassword) {
        alert('Новые пароли не совпадают');
        return;
    }
    
    // Проверяем длину нового пароля
    if (newPassword.length < 1) {
        alert('Новый пароль должен содержать хотя бы 1 символ');
        return;
    }
    
    const formData = {
        current_password: currentPassword,
        new_password: newPassword
    };
    
    try {
        const response = await fetch('/users/change-password', {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        if (response.ok) {
            // Закрываем модальное окно
            document.getElementById('change-password-modal').style.display = 'none';
            
            alert('Пароль успешно изменен');
        } else {
            let errorMessage = 'Неизвестная ошибка';
            try {
                const errorData = await response.json();
                // Исправляем получение сообщения об ошибке
                if (errorData && typeof errorData === 'object') {
                    if (errorData.detail) {
                        errorMessage = errorData.detail;
                    } else if (errorData.message) {
                        errorMessage = errorData.message;
                    } else {
                        // Если detail и message нет, преобразуем объект в строку
                        // Проверяем, является ли errorData объектом с читаемыми свойствами
                        if (typeof errorData === 'object' && errorData !== null) {
                            // Проверяем наличие свойства message или msg
                            if (errorData.message) {
                                errorMessage = errorData.message;
                            } else if (errorData.msg) {
                                errorMessage = errorData.msg;
                            } else {
                                // Пытаемся получить строковое представление объекта ошибки
                                // Проверяем наличие других полей с сообщениями об ошибке
                                if (errorData.error) {
                                    errorMessage = errorData.error;
                                } else if (errorData.msg) {
                                    errorMessage = errorData.msg;
                                } else {
                                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                                }
                            }
                        } else {
                            // Проверяем наличие других полей с сообщениями об ошибке
                            if (errorData && typeof errorData === 'object') {
                                if (errorData.error) {
                                    errorMessage = errorData.error;
                                } else if (errorData.msg) {
                                    errorMessage = errorData.msg;
                                } else {
                                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                                }
                            } else {
                                errorMessage = String(errorData);
                            }
                        }
                    }
                } else {
                    errorMessage = errorData || 'Неизвестная ошибка';
                }
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                try {
                    errorMessage = await response.text() || 'Ошибка при смене пароля';
                } catch (textError) {
                    errorMessage = 'Ошибка при смене пароля';
                }
            }
            alert(`Ошибка при смене пароля: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при смене пароля:', error);
        alert('Ошибка при смене пароля');
    }
});

// Функция открытия модального окна редактирования профиля
function openEditProfileModal() {
    const modal = document.getElementById('edit-profile-modal');
    const nameField = document.getElementById('edit-name');
    const surnameField = document.getElementById('edit-surname');
    const emailField = document.getElementById('edit-email');
    
    // Заполняем поля текущими значениями
    nameField.value = document.getElementById('profile-name').textContent;
    surnameField.value = document.getElementById('profile-surname').textContent;
    emailField.value = document.getElementById('profile-email').textContent;
    
    // Показываем модальное окно
    modal.style.display = 'block';
}

// Закрытие модального окна при клике на крестик
document.querySelector('.close').addEventListener('click', () => {
    document.getElementById('edit-profile-modal').style.display = 'none';
});

// Закрытие модального окна при клике на кнопку "Отмена"
document.querySelector('.cancel-edit').addEventListener('click', () => {
    document.getElementById('edit-profile-modal').style.display = 'none';
});

// Закрытие модального окна при клике вне его области
window.addEventListener('click', (event) => {
    const modal = document.getElementById('edit-profile-modal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
});

// Обработчик отправки формы редактирования профиля
document.getElementById('edit-profile-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const formData = {
        name: document.getElementById('edit-name').value,
        surname: document.getElementById('edit-surname').value,
        email: document.getElementById('edit-email').value
    };
    
    try {
        const response = await fetch('/users/profile', {
            method: 'PUT',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        if (response.ok) {
            const userData = await response.json();
            
            // Обновляем информацию на странице
            document.getElementById('profile-name').textContent = userData.name;
            document.getElementById('profile-surname').textContent = userData.surname;
            document.getElementById('profile-email').textContent = userData.email;
            
            // Закрываем модальное окно
            document.getElementById('edit-profile-modal').style.display = 'none';
            
            alert('Профиль успешно обновлен');
        } else {
            const errorData = await response.json();
            alert(`Ошибка при обновлении профиля: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении профиля:', error);
        alert('Ошибка при обновлении профиля');
    }
});
// Функция для получения ID пользователя из localStorage
function getUserIdFromToken() {
    const userId = localStorage.getItem('user_id');
    if (!userId) return null;
    
    return parseInt(userId);
}

// Функция для отображения заказов пользователя
async function showMyOrders() {
    const userId = getUserIdFromToken();
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch(`/rentals/user/${userId}/with-car-info`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const rentals = await response.json();
            displayRentalsInfo(rentals);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при получении заказов: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при получении заказов:', error);
        alert('Ошибка при получении заказов');
    }
}


// Функция для отображения информации о заказах
function displayRentalsInfo(rentals) {
    // Создаем модальное окно для отображения заказов
    let modal = document.getElementById('my-orders-modal');
    
    if (!modal) {
        // Создаем модальное окно, если его нет
        modal = document.createElement('div');
        modal.id = 'my-orders-modal';
        modal.className = 'modal';
        modal.style.display = 'block';
        modal.innerHTML = `
            <div class="modal-content" style="width: 80%; max-width: 900px;">
                <span class="close-orders-modal">&times;</span>
                <h2>Мои заказы</h2>
                <div id="orders-list">
                    <!-- Список заказов будет добавлен сюда -->
                </div>
            </div>
        `;
        document.body.appendChild(modal);
        
        // Добавляем обработчик для закрытия модального окна
        document.querySelector('.close-orders-modal').addEventListener('click', () => {
            modal.style.display = 'none';
        });
        
        // Закрытие модального окна при клике вне его области
        window.addEventListener('click', (event) => {
            if (event.target === modal) {
                modal.style.display = 'none';
            }
        });
    } else {
        modal.style.display = 'block';
    }
    
    // Очищаем предыдущие данные
    const ordersList = document.getElementById('orders-list');
    ordersList.innerHTML = '';
    
    if (rentals.length === 0) {
        ordersList.innerHTML = '<p>У вас пока нет заказов</p>';
        return;
    }
    
    // Создаем таблицу с информацией о заказах
    const table = document.createElement('table');
    table.className = 'orders-table';
    table.innerHTML = `
        <thead>
            <tr>
                <th>Номер заказа</th>
                <th>Машина</th>
                <th>Номер</th>
                <th>Модель</th>
                <th>Дата начала</th>
                <th>Дата окончания</th>
                <th>Цена</th>
                <th>Статус</th>
            </tr>
        </thead>
        <tbody>
            ${rentals.map(rental => {
                return `
                    <tr>
                        <td>${rental.id}</td>
                        <td>${rental.car_id}</td>
                        <td>${rental.plate_number}</td>
                        <td>${rental.model}</td>
                        <td>${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
                        <td>${rental.ended_at ? new Date(rental.ended_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' }) : (rental.status === 'active' ? 'Активный' : rental.status === 'pending_completion' ? 'Ожидает завершения' : rental.status === 'cancelled' ? 'Отменен' : 'Не завершен')}</td>
                        <td>${rental.price} BYN</td>
                        <td>${rental.status === 'active' ? 'Активный' : rental.status === 'completed' ? 'Завершен' : rental.status === 'cancelled' ? 'Отменен' : rental.status === 'pending_completion' ? 'Ожидает завершения' : rental.status}</td>
                    </tr>
                `;
            }).join('')}
        </tbody>
    `;
    
    ordersList.appendChild(table);
}

// Функция для загрузки водительских прав
async function uploadDriverLicense() {
    // Создаем модальное окно для загрузки водительских прав
    let modal = document.getElementById('upload-driver-license-modal');
    
    if (!modal) {
        // Создаем модальное окно, если его нет
        modal = document.createElement('div');
        modal.id = 'upload-driver-license-modal';
        modal.className = 'modal';
        modal.style.display = 'block';
        modal.innerHTML = `
            <div class="modal-content" style="width: 80%; max-width: 600px;">
                <span class="close-upload-license-modal">&times;</span>
                <h2>Загрузить водительские права</h2>
                <form id="upload-license-form">
                    <div class="form-group">
                        <label for="license-number">Номер прав:</label>
                        <input type="text" id="license-number" name="license_number" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="issued-by">Кем выданы:</label>
                        <input type="text" id="issued-by" name="issued_by" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="expiration-date">Дата окончания:</label>
                        <input type="date" id="expiration-date" name="expiration_date" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="license-photo">Фото прав (лицевая сторона):</label>
                        <input type="file" id="license-photo" name="license_photo" accept="image/*" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="license-photo-back">Фото прав (обратная сторона):</label>
                        <input type="file" id="license-photo-back" name="license_photo_back" accept="image/*" required>
                    </div>
                    
                    <div class="form-actions">
                        <button type="submit" class="btn btn-primary">Загрузить</button>
                        <button type="button" class="btn btn-secondary cancel-upload-license">Отмена</button>
                    </div>
                </form>
            </div>
        `;
        document.body.appendChild(modal);
        
        // Добавляем обработчик для закрытия модального окна
        document.querySelector('.close-upload-license-modal').addEventListener('click', () => {
            modal.style.display = 'none';
        });
        
        // Закрытие модального окна при клике на кнопку "Отмена"
        document.querySelector('.cancel-upload-license').addEventListener('click', () => {
            modal.style.display = 'none';
        });
        
        // Закрытие модального окна при клике вне его области
        window.addEventListener('click', (event) => {
            if (event.target === modal) {
                modal.style.display = 'none';
            }
        });
        
        // Обработчик отправки формы загрузки водительских прав
        document.getElementById('upload-license-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const userId = localStorage.getItem('user_id');
            if (!userId) {
                alert('Пользователь не авторизован');
                return;
            }
            
            const licenseNumber = document.getElementById('license-number').value;
            const issuedBy = document.getElementById('issued-by').value;
            const expirationDate = document.getElementById('expiration-date').value;
            const licensePhoto = document.getElementById('license-photo').files[0];
            const licensePhotoBack = document.getElementById('license-photo-back').files[0];
            
            if (!licensePhoto || !licensePhotoBack) {
                alert('Пожалуйста, загрузите обе фотографии водительских прав');
                return;
            }
            
            try {
                // Загружаем первую фотографию
                const photoFormData = new FormData();
                photoFormData.append('file', licensePhoto);
                photoFormData.append('object_type', 'document');
                photoFormData.append('user_id', userId);
                photoFormData.append('uploaded_by', userId);
                
                const photoResponse = await fetch('/photos/upload', {
                    method: 'POST',
                    headers: {
                        'X-User-ID': userId
                    },
                    body: photoFormData
                });
                
                if (!photoResponse.ok) {
                    const errorData = await photoResponse.json();
                    alert(`Ошибка при загрузке первой фотографии: ${errorData.detail || 'Неизвестная ошибка'}`);
                    return;
                }
                
                const photoData = await photoResponse.json();
                const firstPhotoId = photoData.id;
                
                // Загружаем вторую фотографию
                const photoBackFormData = new FormData();
                photoBackFormData.append('file', licensePhotoBack);
                photoBackFormData.append('object_type', 'document');
                photoBackFormData.append('user_id', userId);
                photoBackFormData.append('uploaded_by', userId);
                
                const photoBackResponse = await fetch('/photos/upload', {
                    method: 'POST',
                    headers: {
                        'X-User-ID': userId
                    },
                    body: photoBackFormData
                });
                
                if (!photoBackResponse.ok) {
                    const errorData = await photoBackResponse.json();
                    alert(`Ошибка при загрузке второй фотографии: ${errorData.detail || 'Неизвестная ошибка'}`);
                    return;
                }
                
                const photoBackData = await photoBackResponse.json();
                const secondPhotoId = photoBackData.id;
                
                // Создаем запись о водительских правах
                const licenseData = {
                    license_number: licenseNumber,
                    issued_by: issuedBy,
                    expiration_date: expirationDate,
                    document_photo_id: firstPhotoId,
                    document_photo_back_id: secondPhotoId,
                    status: 'pending',
                    driver_id: parseInt(userId)  // Привязываем права к пользователю
                };
                
                const licenseResponse = await fetch('/driver_licenses/', {
                    method: 'POST',
                    headers: {
                        'X-User-ID': userId,
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(licenseData)
                });
                
                if (licenseResponse.ok) {
                    alert('Водительские права успешно загружены и отправлены на проверку');
                    modal.style.display = 'none';
                } else {
                    const errorData = await licenseResponse.json();
                    alert(`Ошибка при создании записи о водительских правах: ${errorData.detail || 'Неизвестная ошибка'}`);
                }
            } catch (error) {
                console.error('Ошибка при загрузке водительских прав:', error);
                alert('Ошибка при загрузке водительских прав');
            }
        });
    } else {
        // Если модальное окно уже существует, очищаем форму и показываем его
        document.getElementById('upload-license-form').reset();
        modal.style.display = 'block';
    }
}

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

// Убираем проверку активной аренды при загрузке страницы профиля
// document.addEventListener('DOMContentLoaded', async () => {
//     // Вызываем функцию проверки активной аренды
//     await checkActiveRental();
// });