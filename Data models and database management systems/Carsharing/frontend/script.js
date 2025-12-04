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
    const isAuthenticated = false; // В реальном приложении это будет результатом проверки сессии
    
    const userInfo = document.getElementById('user-info');
    const authButtons = document.getElementById('auth-buttons');
    const logoutButton = document.getElementById('logout-button');
    
    if (isAuthenticated) {
        // Показываем информацию о пользователе
        userInfo.style.display = 'flex';
        authButtons.style.display = 'none';
        logoutButton.style.display = 'block';
        
        // В реальном приложении здесь будут данные из API
        document.getElementById('user-name').textContent = 'Иван Петров';
        document.getElementById('user-cashback').textContent = 'Кэшбэк: 150 BYN';
    } else {
        // Показываем кнопки входа
        userInfo.style.display = 'none';
        authButtons.style.display = 'block';
        logoutButton.style.display = 'none';
    }
}

// Функция для обновления статуса авторизации
function updateAuthStatus(isAuthenticated) {
    const userInfo = document.getElementById('user-info');
    const authButtons = document.getElementById('auth-buttons');
    const logoutButton = document.getElementById('logout-button');
    
    if (isAuthenticated) {
        userInfo.style.display = 'flex';
        authButtons.style.display = 'none';
        logoutButton.style.display = 'block';
        
        // Здесь в реальном приложении будут подгружаться данные пользователя
        document.getElementById('user-name').textContent = 'Иван Петров';
        document.getElementById('user-cashback').textContent = 'Кэшбэк: 150 BYN';
    } else {
        userInfo.style.display = 'none';
        authButtons.style.display = 'block';
        logoutButton.style.display = 'none';
    }
}

// Обработчики для кнопок входа и регистрации
document.querySelector('.login-btn').addEventListener('click', () => {
    // Логика для входа
    console.log('Кнопка входа нажата');
    // В реальном приложении здесь будет вызов API для аутентификации
    // и обновление статуса авторизации
    updateAuthStatus(true); // Для демонстрации
});

document.querySelector('.register-btn').addEventListener('click', () => {
    // Логика для регистрации
    console.log('Кнопка регистрации нажата');
});

document.querySelector('.logout-btn').addEventListener('click', () => {
    // Логика для выхода
    console.log('Кнопка выхода нажата');
    // В реальном приложении здесь будет вызов API для завершения сессии
    // и обновление статуса авторизации
    updateAuthStatus(false); // Для демонстрации
});

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
    getCurrentLocation();
    checkAuthStatus();
});