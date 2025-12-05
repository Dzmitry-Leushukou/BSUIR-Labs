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
            
            // Добавить маркеры для каждой машины, кроме арендованных
            cars.forEach(car => {
                if (car.latitude && car.longitude && car.status !== 'rented') {
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
                        Статус: ${car.status}<br>
                        <button class="rent-car-btn" data-car-id="${car.id}">Арендовать</button>
                    `);
                    
                    // Добавляем обработчик клика для кнопки аренды
                    carMarker.on('popupopen', function() {
                        const rentButton = this._popup._container.querySelector('.rent-car-btn');
                        if (rentButton) {
                            rentButton.addEventListener('click', async function() {
                                const carId = this.getAttribute('data-car-id');
                                await rentCar(carId);
                            });
                        }
                    });
                    
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
    checkAndShowActiveRental(); // Добавляем проверку активной аренды
    
    // Добавляем обработчик для кнопки "Мое местоположение"
    const locateBtn = document.getElementById('locate-user-btn');
    if (locateBtn) {
        locateBtn.addEventListener('click', goToUserLocation);
    }
});

// Функция для аренды автомобиля
async function rentCar(carId) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Для аренды автомобиля необходимо авторизоваться');
        return;
    }
    
    // Получаем ID пользователя из токена (извлекаем из строки токена)
    const userId = parseInt(token.split(':')[0]);
    
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
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify({
                user_id: userId,
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
            showActiveRentalPanel(rentalData);
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
    const token = localStorage.getItem('auth_token');
    if (!token) {
        return null;
    }
    
    // Получаем ID пользователя из токена
    const userId = parseInt(token.split(':')[0]);
    
    try {
        // Запрашиваем все аренды пользователя
        const response = await fetch(`/rentals/user/${userId}`, {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
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
function showActiveRentalPanel(rental) {
    // Если панель уже существует, удаляем её
    const existingPanel = document.getElementById('active-rental-panel');
    if (existingPanel) {
        existingPanel.remove();
    }
    
    if (rental) {
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
        // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточнее UTC
        const nowUTC = new Date(now.getTime() + now.getTimezoneOffset() * 60000);
        
        // Рассчитываем разницу в миллисекундах между текущим временем (в UTC) и началом аренды (в UTC)
        let timeDiff = nowUTC - startedAt;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (1000 * 60)); // Преобразуем миллисекунды в минуты и округляем вниз, чтобы избежать мгновенного округления вверх
        const currentPrice = 1 + minutesDiff * 0.5; // Цена = 1 BYN за начало + 0.5 BYN за минуту
        
        // Проверяем, чтобы цена не была отрицательной или нулевой
        if (currentPrice < 1) {
            currentPrice = 1;
        }
        
        rentalPanel.innerHTML = `
            <div class="rental-info">
                <h3>Текущая аренда</h3>
                <p>Машина: ${rental.car_id}</p>
                <p>Статус: ${rental.status}</p>
                <p>Начало: ${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Minsk' })}</p>
                <p>Текущая цена: ${currentPrice} BYN за ${minutesDiff} мин.</p>
            </div>
            <div class="rental-controls">
                <button id="end-rental-btn" class="btn end-btn">⏹️ Завершить</button>
            </div>
        `;
        
        document.body.appendChild(rentalPanel);
        
        // Добавляем обработчики для кнопок
        document.getElementById('end-rental-btn').addEventListener('click', () => {
            endRental(rental.id);
        });
        
        // Обновляем цену каждые 10 секунд
        const updatePriceInterval = setInterval(() => {
            if (document.getElementById('active-rental-panel')) {
                const updatedStartedAt = new Date(rental.started_at); // Это время в UTC
                const updatedNow = new Date(); // Это текущее время в локальной таймзоне браузера
                
                // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
                // Преобразуем текущее локальное время в его эквивалент в UTC для вычисления разницы
                // Формула: local_time_in_utc = local_time.getTime() + local_timezone_offset_in_ms
                // getTimezoneOffset() возвращает смещение в минутах от UTC к локальному времени, но со знаком минус для таймзон восточнее UTC
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
                
                const priceElement = rentalPanel.querySelector('.rental-info p:nth-child(5)');
                if (priceElement) {
                    priceElement.textContent = `Текущая цена: ${updatedPrice} BYN за ${updatedMinutesDiff} мин.`;
                }
            } else {
                clearInterval(updatePriceInterval);
            }
        }, 1000); // Обновляем каждые 10 секунд
    }
}


// Функция для завершения аренды
async function endRental(rentalId) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
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
                'Authorization': `Bearer ${token}`,
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
                'Authorization': `Bearer ${token}`
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

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        showActiveRentalPanel(activeRental);
    }
}