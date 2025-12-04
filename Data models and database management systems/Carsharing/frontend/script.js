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
    
    const token = localStorage.getItem('auth_token');
    if (!token) {
        console.error('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/cars/all/positions', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
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
                    
                    const carMarker = L.marker([car.latitude, car.longitude], {icon: carIcon}).addTo(map);
                    carMarker.bindPopup(`
                        <b>Машина: ${car.model}</b><br>
                        Номер: ${car.plate_number}<br>
                        Статус: ${car.status}
                    `).openPopup();
                    
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
    // Пока что используем фиктивные данные
    const token = localStorage.getItem('auth_token');
    if (token) {
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
function showUserInfo(userData) {
    const userInfo = document.getElementById('user-info');
    const authButtons = document.getElementById('auth-buttons');
    const logoutButton = document.getElementById('logout-button');
    
    userInfo.style.display = 'flex';
    authButtons.style.display = 'none';
    logoutButton.style.display = 'block';
    
    document.getElementById('user-name').textContent = `${userData.name} ${userData.surname}`;
    document.getElementById('user-cashback').textContent = `Кэшбэк: ${userData.cashback} BYN`;
    
    // Обновляем токен, если он был возвращен с сервера
    if (userData.token) {
        localStorage.setItem('auth_token', userData.token);
    }
    
    // Добавляем обработчик клика на весь прямоугольник профиля
    userInfo.addEventListener('click', (e) => {
        // Проверяем, что клик не был по кнопке "Выйти"
        if (!e.target.classList.contains('logout-btn')) {
            window.location.href = '/profile';
        }
    });
}

// Функция для загрузки информации о пользователе
async function loadUserInfo() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        showAuthButtons();
        return;
    }
    
    try {
        const response = await fetch('/users/profile', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const userData = await response.json();
            showUserInfo(userData);
        } else {
            // Если токен недействителен, удаляем его
            localStorage.removeItem('auth_token');
            showAuthButtons();
        }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
        localStorage.removeItem('auth_token');
        showAuthButtons();
    }
}

// Функция для обновления статуса авторизации
function updateAuthStatus(isAuthenticated, userData = null) {
    if (isAuthenticated && userData) {
        // Сохраняем токен, полученный от сервера, или используем фиктивный для демонстрации
        if (userData.token) {
            localStorage.setItem('auth_token', userData.token);
        }
        showUserInfo(userData);
    } else {
        localStorage.removeItem('auth_token');
        showAuthButtons();
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
                    // Проверяем, есть ли токен в ответе от сервера
                    // В текущей реализации сервер должен возвращать токен
                    if (!userData.token) {
                        // Если сервер не вернул токен, создаем фиктивный для демонстрации
                        userData.token = '1:dummy_token';  // В реальном приложении токен должен возвращаться сервером
                    }
                    updateAuthStatus(true, userData);
                    
                    // После успешного входа обновляем карту с машинами
                    showCarsOnMap();
                } else {
                    const errorData = await response.json();
                    alert(`Ошибка входа: ${errorData.detail || 'Неверный email или пароль'}`);
                }
            } catch (error) {
                console.error('Ошибка при попытке входа:', error);
                alert('Ошибка при попытке входа');
            }
        }
        
        document.body.removeChild(modal);
    });
    
    document.getElementById('cancel-login').addEventListener('click', () => {
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
                    // Registration endpoint now returns user data with token directly
                    if (userData.token) {
                        updateAuthStatus(true, userData);
                        alert('Регистрация и вход прошли успешно!');
                        
                        // После успешной регистрации и входа обновляем карту с машинами
                        showCarsOnMap();
                    } else {
                        // Fallback: try to login after registration
                        const loginResponse = await fetch('/users/login', {
                            method: 'POST',
                            headers: {
                                'Content-Type': 'application/json'
                            },
                            body: JSON.stringify({ email, password })
                        });
                        
                        if (loginResponse.ok) {
                            const loginData = await loginResponse.json();
                            if (!loginData.token) {
                                // Если сервер не вернул токен, создаем фиктивный для демонстрации
                                loginData.token = '1:dummy_token';  // В реальном приложении токен должен возвращаться сервером
                            }
                            updateAuthStatus(true, loginData);
                            alert('Регистрация и вход прошли успешно!');
                            
                            // После успешной регистрации и входа обновляем карту с машинами
                            showCarsOnMap();
                        } else {
                            const errorData = await loginResponse.json();
                            alert(`Ошибка входа после регистрации: ${errorData.detail || 'Неизвестная ошибка'}`);
                        }
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
                }
            } catch (error) {
                console.error('Ошибка при попытке регистрации:', error);
                alert('Ошибка при попытке регистрации');
            }
        }
        
        document.body.removeChild(modal);
    });
    
    document.getElementById('cancel-register').addEventListener('click', () => {
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
    
    // Добавляем обработчик для кнопки "Мое местоположение"
    const locateBtn = document.getElementById('locate-user-btn');
    if (locateBtn) {
        locateBtn.addEventListener('click', goToUserLocation);
    }
});