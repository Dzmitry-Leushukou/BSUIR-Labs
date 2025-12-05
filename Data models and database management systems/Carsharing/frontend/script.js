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
            
            // Добавить маркеры для каждой машины, кроме арендованных и находящихся на обслуживании
            cars.forEach(car => {
                if (car.latitude && car.longitude && car.status === 'available') {
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
    
    // Обновляем токен, если он был возвращен с сервера
    if (userData.token) {
        localStorage.setItem('auth_token', userData.token);
    }
    
    // Проверяем, является ли пользователь администратором
    let isAdmin = false;
    if (userData.role_id) {
        // Если у пользователя есть role_id, проверяем, является ли он админом
        isAdmin = userData.role_id === 1; // admin role ID is 1
    } else {
        // Если role_id нет в userData, запрашиваем информацию о роли
        try {
            const token = localStorage.getItem('auth_token');
            const response = await fetch(`/users/${userData.id}`, {
                method: 'GET',
                headers: {
                    'Authorization': `Bearer ${token}`,
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
            showCompletionModal(rental.id);
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
 
// Функция для отображения модального окна завершения аренды
function showCompletionModal(rentalId) {
    // Проверяем, существует ли уже модальное окно
    const existingModal = document.getElementById('completion-modal');
    if (existingModal) {
        existingModal.remove();
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
            </div>
            
            <!-- Кэшбэк -->
            <div style="margin-bottom: 20px;">
                <h4>Кэшбэк</h4>
                <div style="display: flex; align-items: center; gap: 10px; margin-top: 10px;">
                    <input type="checkbox" id="use-cashback" style="width: 16px; height: 16px;">
                    <label for="use-cashback" style="flex: 1;">Использовать кэшбэк</label>
                    <span id="cashback-amount">Доступно: 0 BYN</span>
                </div>
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
    useCashbackCheckbox.addEventListener('change', function() {
        updateCashbackDisplay();
    });
    
    // Загружаем информацию о пользователе для отображения кэшбэка
    loadUserInfoForCashback();
    
    // Обработчики для кнопок
    document.getElementById('cancel-completion').addEventListener('click', function() {
        document.body.removeChild(modal);
    });
    
    document.getElementById('pay-and-complete').addEventListener('click', function() {
        processPaymentAndComplete(rentalId);
    });
}

// Функция для загрузки информации о пользователе и отображения кэшбэка
async function loadUserInfoForCashback() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
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
            document.getElementById('cashback-amount').textContent = `Доступно: ${userData.cashback} BYN`;
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
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }
    
    try {
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
        
        const minutesDiff = Math.floor(timeDiff / (100 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
        
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
                cashbackUsed = Math.min(totalPrice, userData.cashback);
                finalPrice = totalPrice - cashbackUsed;
                
                if (finalPrice < 0) {
                    finalPrice = 0;
                }
            }
        }
        
        // Создаем запись в логе оплаты
        const paymentLog = {
            rental_id: rentalId,
            user_id: parseInt(token.split(':')[0]),
            pay_type: 'card',
            price: finalPrice,
            ip: await getUserIP()
        };
        
        // Отправляем данные оплаты
        const paymentLogResponse = await fetch('/payment_logs/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify(paymentLog)
        });
        
        if (!paymentLogResponse.ok) {
            throw new Error('Ошибка при создании записи об оплате');
        }
        
        // Обновляем аренду
        const response = await fetch(`/rentals/${rentalId}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify({
                ended_at: endedAt.toISOString(),
                status: "completed",
                price: totalPrice  // Обновляем цену при завершении аренды
            })
        });
        
        if (response.ok) {
            // Если использовался кэшбэк, обновляем баланс пользователя
            if (useCashback && cashbackUsed > 0) {
                await updateCashbackBalance(-cashbackUsed);
            }
            
            // Рассчитываем и добавляем кэшбэк (3% от стоимости поездки)
            const cashbackToAdd = totalPrice * 0.03;
            await updateCashbackBalance(cashbackToAdd);
            
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
            
            alert(`Аренда успешно завершена! С вас списано: ${finalPrice} BYN (1 BYN за начало + ${minutesDiff * 0.5} BYN за ${minutesDiff} минут). Добавлено кэшбэка: ${cashbackToAdd.toFixed(2)} BYN.`);
            
            // Обновляем карту
            showCarsOnMap();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при завершении аренды: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при завершении аренды:', error);
        alert('Ошибка при завершении аренды');
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
    const token = localStorage.getItem('auth_token');
    if (!token) {
        return null;
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
    const token = localStorage.getItem('auth_token');
    if (!token) {
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
                'Authorization': `Bearer ${token}`
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

// Функция для получения IP пользователя
async function getUserIP() {
    try {
        const response = await fetch('https://api.ipify.org?format=json');
        const data = await response.json();
        return data.ip;
    } catch (error) {
        console.error('Ошибка при получении IP:', error);
        return 'unknown';
    }
}
 
// Функция для перехода на админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        showActiveRentalPanel(activeRental);
    }
// Функция для перехода на админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        showActiveRentalPanel(activeRental);
    }
}
}

// Функция для перехода на админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        showActiveRentalPanel(activeRental);
    }
}