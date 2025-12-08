// Инициализация карты
let map;
let marker;
let carMarkers = []; // Массив для хранения маркеров машин

// Функция для получения текущего местоположения пользователя
function getCurrentLocation() {
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
            (position) => {
                const { latitude, longitude } = position.coords;
                
                // Инициализация карты с текущим местоположением
                initMap(latitude, longitude);
                
                // Добавление маркера на карту
                addMarker(latitude, longitude);
                
                // Показать машины на карте
                showCarsOnMap();
            },
            (error) => {
                console.error('Ошибка получения местоположения:', error);
                // Используем Минск как fallback
                initMap(53.904133, 27.557541);
                
                // Показать машины на карте
                showCarsOnMap();
            }
        );
    } else {
        console.error('Геолокация не поддерживается браузером');
        // Используем Минск как fallback
        initMap(53.904133, 27.557541);
        
        // Показать машины на карте
        showCarsOnMap();
    }
}

// Инициализация карты с Leaflet
function initMap(lat, lng) {
    if (map) {
        map.remove();
    }
    
    map = L.map('map').setView([lat, lng], 13);
    
    // Добавление слоя карты OpenStreetMap
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);
}

// Добавление маркера на карту
function addMarker(lat, lng) {
    if (marker) {
        map.removeLayer(marker);
    }
    
    // Создаем маркер с пользовательской иконкой для местоположения пользователя
    const userIcon = L.divIcon({
        className: 'user-location-marker',
        html: '<div style="background-color: #007bff; color: white; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; border: 2px solid white; box-shadow: 0 0 5px rgba(0,0,0,0.5);">👤</div>',
        iconSize: [24, 24],
        iconAnchor: [12, 12]
    });
    
    marker = L.marker([lat, lng], {icon: userIcon}).addTo(map);
    marker.bindPopup('Ваше текущее местоположение').openPopup();
}

// Показать все машины на карте
async function showCarsOnMap() {
    // Очистить предыдущие маркеры машин
    clearCarMarkers();
    
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
            // Теперь бэкенд возвращает только те машины, которые нужно показать
            cars.forEach(car => {
                if (car.latitude && car.longitude) {
                    // Определяем цвет иконки в зависимости от статуса и аренды
                    let iconColor, iconText;
                    if (car.is_rented_by_user) {
                        // Машина арендована текущим пользователем
                        iconColor = '#ff8c00'; // Оранжевый цвет для арендованной машины
                        iconText = '🚗'; // Автомобиль с ключами
                    } else if (car.status === 'available') {
                        // Машина доступна для аренды
                        iconColor = '#28a745'; // Зеленый цвет для доступной машины
                        iconText = '🚗';
                    } else {
                        // Машина недоступна (на обслуживании или другая причина)
                        iconColor = '#dc3545'; // Красный цвет для недоступной машины
                        iconText = '🚗';
                    }
                    
                    // Создаем иконку для маркера машины
                    const carIcon = L.divIcon({
                        className: 'car-marker',
                        html: `<div style="background-color: ${iconColor}; color: white; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; border: 2px solid white; box-shadow: 0 0 5px rgba(0,0.5);">${iconText}</div>`,
                        iconSize: [24, 24],
                        iconAnchor: [12, 12]
                    });
                    
                    const carMarker = L.marker([car.latitude, car.longitude], {icon: carIcon}).addTo(map);
                    
                    // Определяем текст для popup в зависимости от статуса аренды
                    let popupContent;
                    if (car.is_rented_by_user) {
                        popupContent = `
                            <b>Машина: ${car.model}</b><br>
                            Номер: ${car.plate_number}<br>
                            Статус: Арендована вами<br>
                            <span style="color: orange;">🟢 Вы арендовали этот автомобиль</span>
                        `;
                    } else if (car.status === 'available') {
                        popupContent = `
                            <b>Машина: ${car.model}</b><br>
                            Номер: ${car.plate_number}<br>
                            Статус: ${car.status}<br>
                            <button class="rent-car-btn" data-car-id="${car.id}">Арендовать</button>
                        `;
                    } else {
                        popupContent = `
                            <b>Машина: ${car.model}</b><br>
                            Номер: ${car.plate_number}<br>
                            Статус: ${car.status}<br>
                            <span style="color: red;">🔴 Недоступна для аренды</span>
                        `;
                    }
                    
                    carMarker.bindPopup(popupContent);
                    
                    // Добавляем обработчик клика для кнопки аренды, только если машина доступна
                    if (car.status === 'available' && !car.is_rented_by_user) {
                        carMarker.on('popupopen', function() {
                            const rentButton = this._popup._container.querySelector('.rent-car-btn');
                            if (rentButton) {
                                rentButton.addEventListener('click', async function() {
                                    const carId = this.getAttribute('data-car-id');
                                    await rentCar(carId);
                                });
                            }
                        });
                    }
                    
                    carMarkers.push(carMarker);
                }
            });
        } else {
            console.error('Ошибка при получении позиций машин:', response.status);
        }
    } catch (error) {
        console.error('Ошибка при запросе позиций машин:', error);
    }
}

// Очистить маркеры машин с карты
function clearCarMarkers() {
    carMarkers.forEach(marker => {
        map.removeLayer(marker);
    });
    carMarkers = [];
}

// Проверка статуса авторизации пользователя
function checkAuthStatus() {
    // Здесь будет логика проверки сессии
    // Проверяем наличие user_id в localStorage
    const userId = localStorage.getItem('user_id');
    if (userId) {
        loadUserInfo();
    } else {
        showAuthButtons();
    }
}

// Функция для отображения кнопок авторизации
function showAuthButtons() {
    const userInfo = document.getElementById('user-info');
    const authButtons = document.getElementById('auth-buttons');
    const logoutButton = document.getElementById('logout-button');
    
    userInfo.style.display = 'none';
    authButtons.style.display = 'block';
    logoutButton.style.display = 'none';
}

// Функция для отображения информации о пользователе
async function showUserInfo(userData) {
    const userInfo = document.getElementById('user-info');
    const authButtons = document.getElementById('auth-buttons');
    const logoutButton = document.getElementById('logout-button');
    const adminPanelButton = document.getElementById('admin-panel-button');
    
    userInfo.style.display = 'flex';
    authButtons.style.display = 'none';
    logoutButton.style.display = 'block';
    
    document.getElementById('user-name').textContent = `${userData.name} ${userData.surname}`;
    document.getElementById('user-cashback').textContent = `Кэшбэк: ${userData.cashback} BYN`;
    
    // Сохраняем user_id, если он был возвращен с сервера
    if (userData.id) {
        localStorage.setItem('user_id', userData.id);
    }
    
    // Проверяем, является ли пользователь администратором
    let isAdmin = false;
    if (userData.role_id) {
        // Если у пользователя есть role_id, проверяем, является ли он админом
        isAdmin = userData.role_id === 1; // admin role ID is 1
    } else {
        // Если role_id нет в userData, запрашиваем информацию о роли
        try {
            const userId = localStorage.getItem('user_id');
            const response = await fetch(`/users/${userData.id}`, {
                method: 'GET',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                }
            });
            
            if (response.ok) {
                const fullUserData = await response.json();
                isAdmin = fullUserData.role_id === 1;
            }
        } catch (error) {
            console.error('Ошибка при проверке роли пользователя:', error);
        }
    }
    
    // Показываем кнопку админ панели только для администраторов
    if (isAdmin) {
        adminPanelButton.style.display = 'block';
    } else {
        adminPanelButton.style.display = 'none';
    }
    
    // Добавляем обработчик клика на весь прямоугольник профиля
    userInfo.addEventListener('click', (e) => {
        // Проверяем, что клик не был по кнопке "Выйти"
        if (!e.target.classList.contains('logout-btn')) {
            window.location.href = '/profile';
        }
    });
    
    // Проверяем и показываем активную аренду при входе
    setTimeout(checkAndShowActiveRental, 300); // Используем небольшой таймаут для правильного отображения
}

// Функция для загрузки информации о пользователе
async function loadUserInfo() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        showAuthButtons();
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
            showUserInfo(userData);
        } else {
            // Если user_id недействителен, удаляем его
            localStorage.removeItem('user_id');
            showAuthButtons();
        }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
        localStorage.removeItem('user_id');
        showAuthButtons();
    }
}

// Функция для обновления статуса авторизации
function updateAuthStatus(isAuthenticated, userData = null) {
    if (isAuthenticated && userData) {
        // Сохраняем user_id, полученный от сервера
        if (userData.id) {
            localStorage.setItem('user_id', userData.id);
        }
        // Очищаем сохраненные данные форм при успешной аутентификации
        localStorage.removeItem('loginFormData');
        localStorage.removeItem('registerFormData');
        showUserInfo(userData);
        // Проверяем и показываем активную аренду при входе
        setTimeout(checkAndShowActiveRental, 500); // Используем таймаут, чтобы дождаться полной загрузки интерфейса
    } else {
        localStorage.removeItem('user_id');
        showAuthButtons();
        
        // Удаляем панель активной аренды при выходе из аккаунта
        const rentalPanel = document.getElementById('active-rental-panel');
        if (rentalPanel) {
            rentalPanel.remove();
        }
        
        // Скрываем кнопку админ панели при выходе из аккаунта
        const adminPanelButton = document.getElementById('admin-panel-button');
        if (adminPanelButton) {
            adminPanelButton.style.display = 'none';
        }
    }
}

// Обработчики для кнопок входа и регистрации
document.querySelector('.login-btn').addEventListener('click', () => {
    // Создаем модальное окно для входа
    const modal = document.createElement('div');
    modal.style.position = 'fixed';
    modal.style.top = '50%';
    modal.style.left = '50%';
    modal.style.transform = 'translate(-50%, -50%)';
    modal.style.backgroundColor = 'white';
    modal.style.padding = '20px';
    modal.style.borderRadius = '10px';
    modal.style.boxShadow = '0 4px 12px rgba(0,0,0,0.3)';
    modal.style.zIndex = '2000';
    modal.style.display = 'flex';
    modal.style.flexDirection = 'column';
    modal.style.gap = '10px';
    
    modal.innerHTML = `
        <h3>Вход</h3>
        <input type="email" id="login-email" placeholder="Email" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <input type="password" id="login-password" placeholder="Пароль" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <div style="display: flex; gap: 10px;">
            <button id="submit-login" style="padding: 8px 16px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer;">Войти</button>
            <button id="cancel-login" style="padding: 8px 16px; background: #6c757d; color: white; border: none; border-radius: 4px; cursor: pointer;">Отмена</button>
        </div>
    `;
    
    document.body.appendChild(modal);
    
    // Восстанавливаем предыдущие значения из localStorage, если они есть
    const savedLoginData = JSON.parse(localStorage.getItem('loginFormData') || '{}');
    if (savedLoginData.email) {
        document.getElementById('login-email').value = savedLoginData.email;
    }
    if (savedLoginData.password) {
        document.getElementById('login-password').value = savedLoginData.password;
    }
    
    document.getElementById('submit-login').addEventListener('click', async () => {
        const email = document.getElementById('login-email').value;
        const password = document.getElementById('login-password').value;
        
        if (email && password) {
            try {
                const response = await fetch('/users/login', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ email, password })
                });
                
                if (response.ok) {
                    const userData = await response.json();
                    // Проверяем, есть ли user_id в ответе от сервера
                    if (!userData.id) {
                        alert('Ошибка: сервер не вернул идентификатор пользователя');
                        return;
                    }
                    updateAuthStatus(true, userData);
                    
                    // После успешного входа обновляем карту с машинами
                    // Очищаем сохраненные данные формы
                    localStorage.removeItem('loginFormData');
                    
                    // Закрываем модальное окно входа
                    document.body.removeChild(modal);
                    
                    // Проверяем и обновляем статус авторизации
                    checkAuthStatus();
                    
                    showCarsOnMap();
                } else {
                    const errorData = await response.json();
                    alert(`Ошибка входа: ${errorData.detail || 'Неверный email или пароль'}`);
                    // Сохраняем введенные данные при ошибке
                    document.getElementById('login-email').value = email;
                    document.getElementById('login-password').value = password;
                }
            } catch (error) {
                console.error('Ошибка при попытке входа:', error);
                alert('Ошибка при попытке входа');
                // Сохраняем введенные данные при ошибке
                document.getElementById('login-email').value = email;
                document.getElementById('login-password').value = password;
            }
        } else {
            // Если поля не заполнены, сохраняем введенные данные при ошибке
            if (!email) {
                alert('Пожалуйста, введите email');
            }
            if (!password) {
                alert('Пожалуйста, введите пароль');
            }
            // Сохраняем введенные данные
            document.getElementById('login-email').value = email;
            document.getElementById('login-password').value = password;
        }
    });
    
    document.getElementById('cancel-login').addEventListener('click', () => {
        // Сохраняем введенные данные при закрытии модального окна
        const email = document.getElementById('login-email').value;
        const password = document.getElementById('login-password').value;
        // Сохраняем данные в localStorage для возможного восстановления
        localStorage.setItem('loginFormData', JSON.stringify({ email, password }));
        document.body.removeChild(modal);
    });
});

document.querySelector('.register-btn').addEventListener('click', () => {
    // Создаем модальное окно для регистрации
    const modal = document.createElement('div');
    modal.style.position = 'fixed';
    modal.style.top = '50%';
    modal.style.left = '50%';
    modal.style.transform = 'translate(-50%, -50%)';
    modal.style.backgroundColor = 'white';
    modal.style.padding = '20px';
    modal.style.borderRadius = '10px';
    modal.style.boxShadow = '0 4px 12px rgba(0,0,0,0.3)';
    modal.style.zIndex = '2000';
    modal.style.display = 'flex';
    modal.style.flexDirection = 'column';
    modal.style.gap = '10px';
    
    modal.innerHTML = `
        <h3>Регистрация</h3>
        <input type="email" id="register-email" placeholder="Email" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <input type="text" id="register-name" placeholder="Имя" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <input type="text" id="register-surname" placeholder="Фамилия" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <input type="password" id="register-password" placeholder="Пароль" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
        <div style="display: flex; gap: 10px;">
            <button id="submit-register" style="padding: 8px 16px; background: #28a745; color: white; border: none; border-radius: 4px; cursor: pointer;">Зарегистрироваться</button>
            <button id="cancel-register" style="padding: 8px 16px; background: #6c757d; color: white; border: none; border-radius: 4px; cursor: pointer;">Отмена</button>
        </div>
    `;
    
    document.body.appendChild(modal);
    
    // Восстанавливаем предыдущие значения из localStorage, если они есть
    const savedRegisterData = JSON.parse(localStorage.getItem('registerFormData') || '{}');
    if (savedRegisterData.email) {
        document.getElementById('register-email').value = savedRegisterData.email;
    }
    if (savedRegisterData.name) {
        document.getElementById('register-name').value = savedRegisterData.name;
    }
    if (savedRegisterData.surname) {
        document.getElementById('register-surname').value = savedRegisterData.surname;
    }
    if (savedRegisterData.password) {
        document.getElementById('register-password').value = savedRegisterData.password;
    }
    
    document.getElementById('submit-register').addEventListener('click', async () => {
        const email = document.getElementById('register-email').value;
        const name = document.getElementById('register-name').value;
        const surname = document.getElementById('register-surname').value;
        const password = document.getElementById('register-password').value;
        
        if (email && name && surname && password) {
            try {
                const response = await fetch('/users/register', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        email,
                        password,
                        name,
                        surname,
                        cashback: 0,
                        role_id: 2,  // по умолчанию (user role)
                        status: "active"
                    })
                });
                
                if (response.ok) {
                    const userData = await response.json();
                    // Registration endpoint now returns user data with user_id directly
                    if (userData.id) {
                        updateAuthStatus(true, userData);
                        alert('Регистрация и вход прошли успешно!');
                        
                        // После успешной регистрации и входа обновляем карту с машинами
                        // Очищаем сохраненные данные формы
                        localStorage.removeItem('registerFormData');
                        
                        // Закрываем модальное окно регистрации
                        document.body.removeChild(modal);
                        
                        // Проверяем и обновляем статус авторизации
                        checkAuthStatus();
                        
                        showCarsOnMap();
                    } else {
                        alert('Ошибка: сервер не вернул идентификатор пользователя');
                    }
                } else {
                    const errorData = await response.json();
                    if (typeof errorData.detail === 'string') {
                        alert(`Ошибка регистрации: ${errorData.detail}`);
                    } else if (Array.isArray(errorData.detail)) {
                        // Обработка ошибок валидации Pydantic
                        const validationErrors = errorData.detail.map(error => {
                            if (typeof error.msg === 'string') {
                                return error.msg;
                            } else {
                                return JSON.stringify(error.msg);
                            }
                        }).join(', ');
                        alert(`Ошибка регистрации: ${validationErrors}`);
                    } else if (typeof errorData.detail === 'object' && errorData.detail !== null) {
                        // Обработка ошибки, когда detail является объектом
                        alert(`Ошибка регистрации: ${JSON.stringify(errorData.detail)}`);
                    } else {
                        alert(`Ошибка регистрации: ${errorData.detail || 'Неизвестная ошибка'}`);
                    }
                    // Сохраняем введенные данные при ошибке
                    document.getElementById('register-email').value = email;
                    document.getElementById('register-name').value = name;
                    document.getElementById('register-surname').value = surname;
                    document.getElementById('register-password').value = password;
                }
            } catch (error) {
                console.error('Ошибка при попытке регистрации:', error);
                alert('Ошибка при попытке регистрации');
                // Сохраняем введенные данные при ошибке
                document.getElementById('register-email').value = email;
                document.getElementById('register-name').value = name;
                document.getElementById('register-surname').value = surname;
                document.getElementById('register-password').value = password;
            }
        } else {
            // Если поля не заполнены, показываем ошибки и сохраняем введенные данные
            if (!email) {
                alert('Пожалуйста, введите email');
            }
            if (!name) {
                alert('Пожалуйста, введите имя');
            }
            if (!surname) {
                alert('Пожалуйста, введите фамилию');
            }
            if (!password) {
                alert('Пожалуйста, введите пароль');
            }
            // Сохраняем введенные данные
            document.getElementById('register-email').value = email;
            document.getElementById('register-name').value = name;
            document.getElementById('register-surname').value = surname;
            document.getElementById('register-password').value = password;
        }
    });
    
    document.getElementById('cancel-register').addEventListener('click', () => {
        // Сохраняем введенные данные при закрытии модального окна
        const email = document.getElementById('register-email').value;
        const name = document.getElementById('register-name').value;
        const surname = document.getElementById('register-surname').value;
        const password = document.getElementById('register-password').value;
        // Сохраняем данные в localStorage для возможного восстановления
        localStorage.setItem('registerFormData', JSON.stringify({ email, name, surname, password }));
        document.body.removeChild(modal);
    });
});

document.querySelector('.logout-btn').addEventListener('click', () => {
    // Логика для выхода
    console.log('Кнопка выхода нажата');
    // В реальном приложении здесь будет вызов API для завершения сессии
    // и обновление статуса авторизации
    updateAuthStatus(false);
    
    // После выхода очищаем маркеры машин
    clearCarMarkers();
    
    // Удаляем панель активной аренды при выходе из аккаунта
    const rentalPanel = document.getElementById('active-rental-panel');
    if (rentalPanel) {
        rentalPanel.remove();
    }
    
    // Также очищаем сохраненные данные форм при выходе
    localStorage.removeItem('loginFormData');
    localStorage.removeItem('registerFormData');
});

// Функция для перехода к местоположению пользователя на карте
function goToUserLocation() {
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
            (position) => {
                const { latitude, longitude } = position.coords;
                
                // Центрировать карту на текущем местоположении
                map.setView([latitude, longitude], 15);
                
                // Обновить маркер местоположения пользователя
                addMarker(latitude, longitude);
            },
            (error) => {
                console.error('Ошибка получения местоположения:', error);
                alert('Не удалось получить доступ к геолокации. Проверьте настройки браузера.');
            }
        );
    } else {
        console.error('Геолокация не поддерживается браузером');
        alert('Ваш браузер не поддерживает геолокацию');
    }
}

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
    getCurrentLocation();
    checkAuthStatus();
    checkAndShowActiveRental(); // Добавляем проверку активной аренды
    
    // Добавляем обработчик для кнопки "Мое местоположение"
    const locateBtn = document.getElementById('locate-user-btn');
    if (locateBtn) {
        locateBtn.addEventListener('click', goToUserLocation);
    }
});

// Функция для аренды автомобиля
async function rentCar(carId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Для аренды автомобиля необходимо авторизоваться');
        return;
    }
    
    // Проверяем, есть ли у пользователя уже активная аренда
    const activeRental = await getActiveRental();
    if (activeRental) {
        alert('У вас уже есть активная аренда. Завершите её перед тем, как арендовать новую машину.');
        return;
    }
    
    try {
        // Отправляем запрос на создание аренды
        const response = await fetch('/rentals/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-User-ID': userId
            },
            body: JSON.stringify({
                user_id: parseInt(userId),
                car_id: parseInt(carId),
                started_at: new Date(new Date().toLocaleString("en-US", {timeZone: "Europe/Minsk"})).toISOString(), // Устанавливаем время с учетом часового пояса Минска
                price: 1, // Начальная цена 1 BYN
                status: "active"
            })
        });
        
        if (response.ok) {
            const rentalData = await response.json();
            alert(`Автомобиль успешно арендован! ID аренды: ${rentalData.id}`);
            
            // Обновляем статус машины на "rented" визуально на карте
            updateCarMarkerStatus(carId, "rented");
            
            // Показываем панель активной аренды сразу после аренды
            await showActiveRentalPanel(rentalData);
            
            // Обновляем карту, чтобы отобразить только арендованную машину
            showCarsOnMap();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при аренде автомобиля: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при попытке арендовать автомобиль:', error);
        alert('Ошибка при попытке арендовать автомобиль');
    }
}

// Функция для обновления статуса маркера машины
function updateCarMarkerStatus(carId, newStatus) {
    // Перезагружаем все маркеры, чтобы отразить изменения статуса
    showCarsOnMap();
}

// Функция для получения активной аренды пользователя
async function getActiveRental() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        return null;
    }
    
    try {
        // Запрашиваем все аренды пользователя с информацией о машинах
        const response = await fetch(`/rentals/user/${userId}/with-car-info`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const rentals = await response.json();
            // Находим активную аренду (если есть)
            const activeRental = rentals.find(rental => rental.status === 'active');
            return activeRental || null;
        } else {
            console.error('Ошибка при получении аренды:', response.status);
            return null;
        }
    } catch (error) {
        console.error('Ошибка при запросе аренды:', error);
        return null;
    }
}

// Функция для отображения панели активной аренды
async function showActiveRentalPanel(rental) {
    // Если панель уже существует, удаляем её
    const existingPanel = document.getElementById('active-rental-panel');
    if (existingPanel) {
        existingPanel.remove();
    }
    
    if (rental) {
        // Загружаем информацию о машине, связанной с арендой
        const userId = localStorage.getItem('user_id');
        let rentalWithCarInfo = rental;
        
        // Запрашиваем расширенную информацию об аренде с информацией о машине
        try {
            const response = await fetch(`/rentals/user/${userId}/with-car-info?limit=100`, {
                method: 'GET',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                }
            });
            
            if (response.ok) {
                const rentalsWithCarInfo = await response.json();
                rentalWithCarInfo = rentalsWithCarInfo.find(r => r.id === rental.id) || rental;
            }
        } catch (error) {
            console.error('Ошибка при получении информации об аренде с данными автомобиля:', error);
            // Используем базовую информацию об аренде, если не удалось получить расширенную
        }
        
        // Создаем панель активной аренды
        const rentalPanel = document.createElement('div');
        rentalPanel.id = 'active-rental-panel';
        rentalPanel.className = 'active-rental-panel';
        
        // Рассчитываем текущую цену аренды
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Используем объекты Date напрямую для корректного вычисления разницы
        const startedAt = new Date(rental.started_at); // Это время в UTC
        const now = new Date(); // Это текущее время в локальной таймзоне браузера
        
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
        // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
        // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточее UTC
        const nowUTC = new Date(now.getTime() + now.getTimezoneOffset() * 60000);
        
        // Рассчитываем разницу в миллисекундах между текущим временем (в UTC) и началом аренды (в UTC)
        let timeDiff = nowUTC - startedAt;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (100 * 60)); // Преобразуем миллисекунды в минуты и округляем вниз, чтобы избежать мгновенного округления вверх
        const currentPrice = 1 + minutesDiff * 0.5; // Цена = 1 BYN за начало + 0.5 BYN за минуту
        
        // Проверяем, чтобы цена не была отрицательной или нулевой
        if (currentPrice < 1) {
            currentPrice = 1;
        }
        
        rentalPanel.innerHTML = `
            <div class="rental-info">
                <h3>Текущая аренда</h3>
                <div>
                    <p><strong>Модель:</strong> ${rentalWithCarInfo.model || rental.car_id}</p>
                    <p><strong>Номер:</strong> ${rentalWithCarInfo.plate_number || 'Неизвестен'}</p>
                    <p><strong>Статус:</strong> ${rental.status}</p>
                </div>
                <p><strong>Начало:</strong> ${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</p>
                <p id="rental-price-${rental.id}">Текущая цена: ${currentPrice} BYN за ${minutesDiff} мин.</p>
            </div>
            <div class="rental-controls">
                <button id="end-rental-btn" class="btn end-btn">⏹️ Завершить</button>
            </div>
        `;
        
        document.body.appendChild(rentalPanel);
        
        // Добавляем обработчики для кнопок
        document.getElementById('end-rental-btn').addEventListener('click', () => {
            showCompletionModal(rental.id);
        });
        
        // Обновляем цену каждую секунду для реального времени
        const updatePriceInterval = setInterval(() => {
            if (document.getElementById('active-rental-panel')) {
                const updatedStartedAt = new Date(rental.started_at); // Это время в UTC
                const updatedNow = new Date(); // Это текущее время в локальной таймзоне браузера
                
                // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
                // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
                // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
                // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточее UTC
                const updatedNowUTC = new Date(updatedNow.getTime() + updatedNow.getTimezoneOffset() * 60000);
                
                // Рассчитываем разницу в миллисекундах между текущим временем (в UTC) и началом аренды (в UTC)
                let updatedTimeDiff = updatedNowUTC - updatedStartedAt;
                if (updatedTimeDiff < 0) {
                    // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
                    updatedTimeDiff = 0;
                }
                
                const updatedMinutesDiff = Math.floor(updatedTimeDiff / (1000 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
                const updatedPrice = 1 + updatedMinutesDiff * 0.5; // Цена = 1 BYN за начало + 0.5 BYN за минуту
                
                // Проверяем, чтобы цена не была отрицательной или нулевой
                if (updatedPrice < 1) {
                    updatedPrice = 1;
                }
                
                // Обновляем цену в элементе с уникальным ID
                const priceElement = document.getElementById(`rental-price-${rental.id}`);
                if (priceElement) {
                    priceElement.textContent = `Текущая цена: ${updatedPrice} BYN за ${updatedMinutesDiff} мин.`;
                }
            } else {
                clearInterval(updatePriceInterval);
            }
        }, 1000); // Обновляем каждую секунду для реального времени
    }
}


// Функция для завершения аренды
async function endRental(rentalId) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Необходима авторизация для управления арендой');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите завершить аренду?')) {
        return;
    }
    
    try {
        // Получаем текущую аренду для расчета цены
        const rentalResponse = await fetch(`/rentals/${rentalId}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (!rentalResponse.ok) {
            throw new Error('Не удалось получить информацию об аренде');
        }
        
        const rental = await rentalResponse.json();
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Используем объекты Date напрямую для корректного вычисления разницы
        const startedAt = new Date(rental.started_at); // Это время в UTC
        const endedAt = new Date(); // Это текущее время в локальной таймзоне браузера
        
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
        // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
        // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточнее UTC
        const endedAtUTC = new Date(endedAt.getTime() + endedAt.getTimezoneOffset() * 60000);
        
        // Рассчитываем разницу в миллисекундах между окончанием аренды и началом
        let timeDiff = endedAtUTC - startedAt;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (1000 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
        
        // Цена = 1 BYN за начало + 0.5 BYN за минуту
        const price = 1 + minutesDiff * 0.5;
        
        // Проверяем, чтобы цена не была отрицательной или нулевой
        if (price < 1) {
            price = 1;
        }
        
        const response = await fetch(`/rentals/${rentalId}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
                'X-User-ID': userId
            },
            body: JSON.stringify({
                ended_at: endedAt.toISOString(),
                status: "completed",
                price: price  // Обновляем цену при завершении аренды
            })
        });
        
        if (response.ok) {
            const updatedRental = await response.json();
            // Удаляем панель активной аренды
            const rentalPanel = document.getElementById('active-rental-panel');
            if (rentalPanel) {
                rentalPanel.remove();
            }
            alert(`Аренда успешно завершена! С вас списано: ${price} BYN (1 BYN за начало + ${minutesDiff * 0.5} BYN за ${minutesDiff} минут).`);
            // Обновляем карту
            showCarsOnMap();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при завершении аренды: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при попытке завершить аренду:', error);
        alert('Ошибка при попытке завершить аренду');
    }
}
 
// Функция для отображения модального окна завершения аренды
function showCompletionModal(rentalId) {
    // Проверяем, существует ли уже модальное окно
    const existingModal = document.getElementById('completion-modal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // Получаем информацию о текущей аренде для расчета цены
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }
    
    // Асинхронно получаем информацию об аренде и рассчитываем цену
    fetch(`/rentals/${rentalId}`, {
        method: 'GET',
        headers: {
            'X-User-ID': userId,
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.json())
    .then(rental => {
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Используем объекты Date напрямую для корректного вычисления разницы
        const startedAt = new Date(rental.started_at); // Это время в UTC
        const now = new Date(); // Это текущее время в локальной таймзоне браузера
        
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
        // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
        // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточее UTC
        const nowUTC = new Date(now.getTime() + now.getTimezoneOffset() * 60000);
        
        // Рассчитываем разницу в миллисекундах между текущим временем (в UTC) и началом аренды (в UTC)
        let timeDiff = nowUTC - startedAt;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (1000 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
        const currentPrice = 1 + minutesDiff * 0.5; // Цена = 1 BYN за начало + 0.5 BYN за минуту
        
        // Проверяем, чтобы цена не была отрицательной или нулевой
        if (currentPrice < 1) {
            currentPrice = 1;
        }
        
        // Создаем модальное окно
        const modal = document.createElement('div');
        modal.id = 'completion-modal';
        modal.style.cssText = `
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.5);
            display: flex;
            justify-content: center;
            align-items: center;
            z-index: 10000;
        `;
        
        modal.innerHTML = `
            <div style="
                background: white;
                padding: 20px;
                border-radius: 10px;
                width: 90%;
                max-width: 500px;
                position: relative;
                box-shadow: 0 4px 12px rgba(0,0,0.3);
            ">
                <h3>Завершение аренды</h3>
                
                <!-- Информация о поездке -->
                <div style="margin-bottom: 20px; padding: 15px; background-color: #f8f9fa; border-radius: 5px;">
                    <h4>Информация о поездке</h4>
                    <p><strong>Начало аренды:</strong> ${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</p>
                    <p><strong>Текущая стоимость:</strong> <span id="current-trip-price">${currentPrice} BYN</span> за ${minutesDiff} мин.</p>
                </div>
                
                <!-- Фотографии -->
                <div style="margin-bottom: 20px;">
                    <h4>Добавить фотографии</h4>
                    <div id="photo-preview-container" style="display: flex; flex-wrap: wrap; gap: 10px; margin-top: 10px;"></div>
                    <input type="file" id="photo-upload" accept="image/*" multiple style="margin-top: 10px;">
                </div>
                
                <!-- Оплата -->
                <div style="margin-bottom: 20px;">
                    <h4>Оплата</h4>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 10px;">
                        <input type="text" id="card-number" placeholder="Номер карты" maxlength="19" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
                        <input type="text" id="card-holder" placeholder="Имя держателя" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
                        <input type="text" id="expiry-date" placeholder="ММ/ГГ" maxlength="5" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
                        <input type="text" id="cvv" placeholder="CVV" maxlength="3" style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
                    </div>
                
                <!-- Кэшбэк -->
                <div style="margin-bottom: 20px;">
                    <h4>Кэшбэк</h4>
                    <div style="display: flex; align-items: center; gap: 10px; margin-top: 10px;">
                        <input type="checkbox" id="use-cashback" style="width: 16px; height: 16px;">
                        <label for="use-cashback" style="flex: 1;">Использовать кэшбэк</label>
                        <span id="cashback-amount">Доступно: 0 BYN</span>
                    </div>
                    <div id="cashback-input-container" style="margin-top: 10px; display: none;">
                        <input type="number" id="cashback-amount-input" placeholder="Сумма кэшбэка" min="0" step="0.01" style="width: 100%; padding: 8px; border: 1px solid #ccc; border-radius: 4px;">
                        <div style="margin-top: 5px; font-size: 0.8em; color: #666;">Максимум можно использовать: <span id="max-cashback-amount">0</span> BYN</div>
                    </div>
                
                <div style="display: flex; gap: 10px; margin-top: 20px;">
                    <button id="cancel-completion" style="
                        flex: 1;
                        padding: 10px;
                        background: #6c757d;
                        color: white;
                        border: none;
                        border-radius: 5px;
                        cursor: pointer;
                    ">Отмена</button>
                    <button id="pay-and-complete" style="
                        flex: 1;
                        padding: 10px;
                        background: #28a745;
                        color: white;
                        border: none;
                        border-radius: 5px;
                        cursor: pointer;
                    ">Оплатить и завершить</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(modal);
        
        // Добавляем обработчики для элементов формы
        const photoUpload = document.getElementById('photo-upload');
        const photoPreviewContainer = document.getElementById('photo-preview-container');
        
        // Обработчик для загрузки фотографий
        photoUpload.addEventListener('change', function(e) {
            const files = e.target.files;
            photoPreviewContainer.innerHTML = ''; // Очищаем предыдущие превью
            
            for (let i = 0; i < files.length; i++) {
                const file = files[i];
                if (file.type.startsWith('image/')) {
                    const reader = new FileReader();
                    
                    reader.onload = function(e) {
                        const img = document.createElement('img');
                        img.src = e.target.result;
                        img.style.width = '80px';
                        img.style.height = '80px';
                        img.style.objectFit = 'cover';
                        img.style.border = '1px solid #ddd';
                        img.style.borderRadius = '4px';
                        photoPreviewContainer.appendChild(img);
                    };
                    
                    reader.readAsDataURL(file);
                }
            }
        });
        
        // Форматирование номера карты
        const cardNumberInput = document.getElementById('card-number');
        cardNumberInput.addEventListener('input', function(e) {
            let value = e.target.value.replace(/\D/g, ''); // Убираем все нецифровые символы
            let formattedValue = '';
            
            for (let i = 0; i < value.length; i++) {
                if (i > 0 && i % 4 === 0) {
                    formattedValue += ' ';
                }
                formattedValue += value[i];
            }
            
            e.target.value = formattedValue;
        });
        
        // Форматирование даты
        const expiryDateInput = document.getElementById('expiry-date');
        expiryDateInput.addEventListener('input', function(e) {
            let value = e.target.value.replace(/\D/g, ''); // Убираем все нецифровые символы
            
            if (value.length > 2) {
                value = value.substring(0, 2) + '/' + value.substring(2, 4);
            }
            
            e.target.value = value;
        });
        
        // Валидация CVV
        const cvvInput = document.getElementById('cvv');
        cvvInput.addEventListener('input', function(e) {
            e.target.value = e.target.value.replace(/\D/g, '').substring(0, 3);
        });
        
        // Обработчик для чекбокса кэшбэка
        const useCashbackCheckbox = document.getElementById('use-cashback');
        const cashbackInputContainer = document.getElementById('cashback-input-container');
        const cashbackAmountInput = document.getElementById('cashback-amount-input');
        const maxCashbackAmountSpan = document.getElementById('max-cashback-amount');
        
        useCashbackCheckbox.addEventListener('change', function() {
            updateCashbackDisplay();
            if (this.checked) {
                cashbackInputContainer.style.display = 'block';
                cashbackAmountInput.value = ''; // Очищаем поле при отображении
            } else {
                cashbackInputContainer.style.display = 'none';
                cashbackAmountInput.value = ''; // Очищаем поле при скрытии
            }
        });
        
        // Загружаем информацию о пользователе для отображения кэшбэка
        loadUserInfoForCashback();
        
        // Обработчик для поля ввода суммы кэшбэка
        cashbackAmountInput.addEventListener('input', function() {
            const userData = getUserData();
            userData.then(user => {
                if (user) {
                    const maxCashback = Math.min(user.cashback, currentPrice);
                    const inputValue = parseFloat(this.value);
                    
                    if (inputValue > maxCashback) {
                        this.value = maxCashback;
                    }
                    
                    if (inputValue < 0) {
                        this.value = 0;
                    }
                }
            });
        });
        
        // Обработчики для кнопок
        document.getElementById('cancel-completion').addEventListener('click', function() {
            document.body.removeChild(modal);
        });
        
        document.getElementById('pay-and-complete').addEventListener('click', function() {
            processPaymentAndComplete(rentalId);
        });
    })
    .catch(error => {
        console.error('Ошибка при получении информации об аренде:', error);
        alert('Ошибка при получении информации об аренде');
    });
}

// Функция для загрузки информации о пользователе и отображения кэшбэка
async function loadUserInfoForCashback() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
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
            document.getElementById('cashback-amount').textContent = `Доступно: ${userData.cashback} BYN`;
            document.getElementById('max-cashback-amount').textContent = userData.cashback;
        }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
    }
}

// Функция для обновления отображения кэшбэка
function updateCashbackDisplay() {
    const useCashbackCheckbox = document.getElementById('use-cashback');
    const cashbackAmountElement = document.getElementById('cashback-amount');
    
    if (useCashbackCheckbox.checked) {
        cashbackAmountElement.style.color = '#28a745';
        cashbackAmountElement.style.fontWeight = 'bold';
    } else {
        cashbackAmountElement.style.color = '';
        cashbackAmountElement.style.fontWeight = '';
    }
}

// Функция для обработки оплаты и завершения аренды
async function processPaymentAndComplete(rentalId) {
    // Получаем текущую информацию о аренде
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }
    
    try {
        const rentalResponse = await fetch(`/rentals/${rentalId}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (!rentalResponse.ok) {
            throw new Error('Не удалось получить информацию об аренде');
        }
        
        const rental = await rentalResponse.json();
        
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Используем объекты Date напрямую для корректного вычисления разницы
        const startedAt = new Date(rental.started_at); // Это время в UTC
        const endedAt = new Date(); // Это текущее время в локальной таймзоне браузера
        
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
        // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
        // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточнее UTC
        const endedAtUTC = new Date(endedAt.getTime() + endedAt.getTimezoneOffset() * 60000);
        
        // Рассчитываем разницу в миллисекундах между окончанием аренды и началом
        let timeDiff = endedAtUTC - startedAt;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (1000 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
        
        // Цена = 1 BYN за начало + 0.5 BYN за минуту
        const totalPrice = 1 + minutesDiff * 0.5;
        
        // Проверяем, чтобы цена не была отрицательной или нулевой
        if (totalPrice < 1) {
            totalPrice = 1;
        }
        
        // Валидация данных карты
        const cardNumber = document.getElementById('card-number').value.replace(/\s/g, '');
        const cardHolder = document.getElementById('card-holder').value;
        const expiryDate = document.getElementById('expiry-date').value;
        const cvv = document.getElementById('cvv').value;
        
        if (!validateCardData(cardNumber, cardHolder, expiryDate, cvv)) {
            return;
        }
        
        // Проверяем, использовать ли кэшбэк
        const useCashback = document.getElementById('use-cashback').checked;
        let finalPrice = totalPrice;
        let cashbackUsed = 0;
        
        if (useCashback) {
            const userData = await getUserData();
            if (userData.cashback > 0) {
                // Получаем выбранную сумму кэшбэка
                const cashbackInputValue = document.getElementById('cashback-amount-input').value;
                if (cashbackInputValue) {
                    const requestedCashback = parseFloat(cashbackInputValue);
                    
                    // Проверяем, что запрашиваемая сумма не превышает доступный кэшбэк и не больше общей цены
                    if (requestedCashback > userData.cashback) {
                        alert(`Вы не можете использовать кэшбэк больше, чем у вас есть. Максимум доступно: ${userData.cashback} BYN`);
                        return;
                    }
                    
                    if (requestedCashback > totalPrice) {
                        alert(`Вы не можете использовать кэшбэк больше, чем стоимость поездки. Максимум доступно: ${totalPrice} BYN`);
                        return;
                    }
                    
                    if (requestedCashback < 0) {
                        alert('Сумма кэшбэка не может быть отрицательной');
                        return;
                    }
                    
                    cashbackUsed = requestedCashback;
                    finalPrice = totalPrice - cashbackUsed;
                    
                    if (finalPrice < 0) {
                        finalPrice = 0;
                    }
                } else {
                    // Если поле пустое, используем минимальное значение из доступного кэшбэка и общей цены
                    cashbackUsed = Math.min(totalPrice, userData.cashback);
                    finalPrice = totalPrice - cashbackUsed;
                    
                    if (finalPrice < 0) {
                        finalPrice = 0;
                    }
                }
            }
        }
        
        // Создаем запись в логе оплаты
        const paymentLog = {
            rental_id: rentalId,
            user_id: parseInt(userId),
            pay_type: 'card',
            price: finalPrice
        };
        
        // Отправляем данные оплаты
        const paymentLogResponse = await fetch('/payment_logs/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-User-ID': userId
            },
            body: JSON.stringify(paymentLog)
        });
        
        if (!paymentLogResponse.ok) {
            throw new Error('Ошибка при создании записи об оплате');
        }
        
        // Проверяем, загружены ли фотографии (минимум 1 обязательно)
        const photoUpload = document.getElementById('photo-upload');
        const files = photoUpload.files;
        
        if (files.length === 0) {
            alert('Пожалуйста, загрузите хотя бы одну фотографию завершения поездки.');
            return;
        }
        
        // Загружаем первую фотографию и создаем запрос на подтверждение завершения поездки
        let completionPhotoId = null;
        for (let i = 0; i < files.length; i++) {
            const file = files[i];
            if (file.type.startsWith('image/')) {
                const photoId = await uploadPhoto(file, rental.car_id, parseInt(userId));
                if (photoId) {
                    completionPhotoId = photoId; // Используем первую загруженную фотографию
                    break;
                }
            }
        }
        
        if (!completionPhotoId) {
            alert('Не удалось загрузить фотографии. Пожалуйста, попробуйте снова.');
            return;
        }
        
        // Создаем запрос на подтверждение завершения поездки
        const tripCompletionResponse = await fetch('/trip-completions/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-User-ID': userId
            },
            body: JSON.stringify({
                rental_id: rentalId,
                completion_photo_id: completionPhotoId
            })
        });
        
        if (tripCompletionResponse.ok) {
            // Если использовался кэшбэк, обновляем баланс пользователя
            if (useCashback && cashbackUsed > 0) {
                await updateCashbackBalance(-cashbackUsed);
                // Обновляем информацию о пользователе (включая кэшбэк)
                await loadUserInfo();
            }
            
            // Рассчитываем и добавляем кэшбэк (3% от стоимости поездки)
            const cashbackToAdd = totalPrice * 0.03;
            await updateCashbackBalance(cashbackToAdd);
            // Обновляем информацию о пользователе (включая кэшбэк)
            await loadUserInfo();
            
            // Удаляем панель активной аренды
            const rentalPanel = document.getElementById('active-rental-panel');
            if (rentalPanel) {
                rentalPanel.remove();
            }
            
            // Закрываем модальное окно
            const modal = document.getElementById('completion-modal');
            if (modal) {
                modal.remove();
            }
            
            alert(`Запрос на завершение аренды отправлен! Ожидайте подтверждения администратором. С вас списано: ${finalPrice} BYN (1 BYN за начало + ${minutesDiff * 0.5} BYN за ${minutesDiff} минут). Добавлено кэшбэка: ${cashbackToAdd.toFixed(2)} BYN.`);
            
            // Обновляем информацию о пользователе (включая кэшбэк)
            loadUserInfo();
            
            // Обновляем карту
            showCarsOnMap();
        } else {
            const errorData = await tripCompletionResponse.json();
            alert(`Ошибка при отправке запроса на завершение аренды: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при завершении аренды:', error);
        alert('Ошибка при завершении аренды');
    }
}

// Функция для загрузки фотографии
async function uploadPhoto(file, carId, userId) {
    const currentUserId = localStorage.getItem('user_id');
    if (!currentUserId) {
        console.error('Необходима авторизация для загрузки фотографий');
        return null;
    }
    
    try {
        // Создаем FormData для отправки файла
        const formData = new FormData();
        formData.append('file', file);
        formData.append('object_type', 'car');
        formData.append('car_id', carId);
        formData.append('user_id', currentUserId);
        formData.append('uploaded_by', currentUserId);
        
        // Отправляем запрос на загрузку фотографии
        const response = await fetch('/photos/upload', {
            method: 'POST',
            headers: {
                'X-User-ID': currentUserId
            },
            body: formData
        });
        
        if (response.ok) {
            const photoData = await response.json();
            return photoData.id; // Возвращаем ID загруженной фотографии
        } else {
            const errorData = await response.json();
            console.error('Ошибка при загрузке фотографии:', errorData.detail || 'Неизвестная ошибка');
            return null;
        }
    } catch (error) {
        console.error('Ошибка при загрузке фотографии:', error);
        return null;
    }
}

// Функция для валидации данных карты
function validateCardData(cardNumber, cardHolder, expiryDate, cvv) {
    // Проверка номера карты (16 цифр)
    const cardNumberRegex = /^\d{16}$/;
    if (!cardNumberRegex.test(cardNumber)) {
        alert('Неверный формат номера карты. Должно быть 16 цифр.');
        return false;
    }
    
    // Проверка имени держателя
    if (!cardHolder.trim()) {
        alert('Введите имя держателя карты.');
        return false;
    }
    
    // Проверка срока действия
    const expiryRegex = /^(0[1-9]|1[0-2])\/?([0-9]{2})$/;
    if (!expiryRegex.test(expiryDate)) {
        alert('Неверный формат срока действия. Используйте ММ/ГГ.');
        return false;
    }
    
    // Проверка CVV
    const cvvRegex = /^\d{3}$/;
    if (!cvvRegex.test(cvv)) {
        alert('Неверный формат CVV. Должно быть 3 цифры.');
        return false;
    }
    
    return true;
}

// Функция для получения данных пользователя
async function getUserData() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        return null;
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
            return await response.json();
        }
        return null;
    } catch (error) {
        console.error('Ошибка при получении данных пользователя:', error);
        return null;
    }
}

// Функция для обновления баланса кэшбэка
async function updateCashbackBalance(amount) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        return;
    }
    
    try {
        const userData = await getUserData();
        if (!userData) {
            return;
        }
        
        const newCashback = userData.cashback + amount;
        
        // Обновляем данные пользователя
        const response = await fetch(`/users/${userData.id}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
                'X-User-ID': userId
            },
            body: JSON.stringify({
                cashback: newCashback
            })
        });
        
        if (!response.ok) {
            console.error('Ошибка при обновлении кэшбэка:', response.status);
        }
    } catch (error) {
        console.error('Ошибка при обновлении кэшбэка:', error);
    }
}

// Function to get user IP has been removed as per requirements
 

// Функция для перехода на админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        await showActiveRentalPanel(activeRental);
    }
}