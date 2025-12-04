// Инициализация карты
let map;
let marker;

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
            },
            (error) => {
                console.error('Ошибка получения местоположения:', error);
                // Используем Минск как fallback
                initMap(53.904133, 27.557541);
            }
        );
    } else {
        console.error('Геолокация не поддерживается браузером');
        // Используем Минск как fallback
        initMap(53.904133, 27.557541);
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
    
    marker = L.marker([lat, lng]).addTo(map);
    marker.bindPopup('Ваше текущее местоположение').openPopup();
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
        localStorage.setItem('auth_token', userData.token || 'dummy_token');
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
                    // Добавляем временный токен для демонстрации
                    userData.token = 'dummy_token';
                    updateAuthStatus(true, userData);
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
                        role_id: 1,  // по умолчанию
                        status: "active"
                    })
                });
                
                if (response.ok) {
                    alert('Регистрация прошла успешно! Теперь вы можете войти.');
                } else {
                    const errorData = await response.json();
                    alert(`Ошибка регистрации: ${errorData.detail || 'Неизвестная ошибка'}`);
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
});

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
    getCurrentLocation();
    checkAuthStatus();
});