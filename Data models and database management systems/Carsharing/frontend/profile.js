// Функция для возврата на предыдущую страницу
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
            
            // Заполняем информацию о пользователе
            document.getElementById('profile-name').textContent = userData.name;
            document.getElementById('profile-surname').textContent = userData.surname;
            document.getElementById('profile-email').textContent = userData.email;
            document.getElementById('profile-cashback').textContent = `${userData.cashback} BYN`;
            
            // Получаем информацию о роли пользователя из базы данных
            const roleResponse = await fetch(`/roles/${userData.role_id}`);
            if (roleResponse.ok) {
                const roleData = await roleResponse.json();
                document.getElementById('profile-role').textContent = roleData.name;
            } else {
                // В случае ошибки используем резервный вариант
                const roleName = userData.role_id === 1 ? 'Администратор' :
                               userData.role_id === 2 ? 'Пользователь' :
                               userData.role_id === 3 ? 'Менеджер' : 'Неизвестная роль';
                document.getElementById('profile-role').textContent = roleName;
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
                if (userData.role_id === 1) {  // admin role ID is 1
                    adminPanelButton.style.display = 'block';
                } else {
                    adminPanelButton.style.display = 'none';
                }
            }
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
                        errorMessage = JSON.stringify(errorData);
                    }
                } else {
                    errorMessage = errorData || 'Неизвестная ошибка';
                }
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                errorMessage = await response.text() || 'Ошибка при смене пароля';
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
                <th>ID заказа</th>
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
                        <td>${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</td>
                        <td>${rental.ended_at ? new Date(rental.ended_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' }) : (rental.status === 'completed' ? 'Не указана' : 'Активный')}</td>
                        <td>${rental.price} BYN</td>
                        <td>${rental.status}</td>
                    </tr>
                `;
            }).join('')}
        </tbody>
    `;
    
    ordersList.appendChild(table);
}

// Убираем проверку активной аренды при загрузке страницы профиля
// document.addEventListener('DOMContentLoaded', async () => {
//     // Вызываем функцию проверки активной аренды
//     await checkActiveRental();
// });