// Функция для проверки статуса пользователя
async function checkUserStatus() {
    const userData = getUserData();
    if (!userData) {
        return null;
    }

    try {
        const response = await authenticatedFetch('/users/profile');
        if (response.ok) {
            const userData = await response.json();
            return userData.status;
        }
        return null;
    } catch (error) {
        console.error('Ошибка при проверке статуса пользователя:', error);
        return null;
    }
}

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

    const userData = getUserData();
    if (!userData) {
        // Пользователь не авторизован - просто не показываем машины
        return;
    }

    // Проверяем, не забанен ли пользователь
    if (userData.status === 'banned') {
        // Если пользователь забанен, не показываем машины
        return;
    }

    try {
        const response = await authenticatedFetch('/cars/all/positions');

        if (response.ok) {
            const cars = await response.json();

            // Проверяем, что карта инициализирована
            if (typeof map === 'undefined' || !map) {
                console.warn('Карта ещё не инициализирована, пропускаем обновление маркеров');
                return;
            }

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
                            Статус: ${car.status === 'available' ? 'Доступен' : car.status === 'rented' ? 'Арендован' : car.status === 'maintenance' ? 'На обслуживании' : car.status === 'pending_completion' ? 'Ожидает завершения' : car.status}<br>
                            <button class="rent-car-btn" data-car-id="${car.id}">Арендовать</button>
                        `;
                    } else {
                        popupContent = `
                            <b>Машина: ${car.model}</b><br>
                            Номер: ${car.plate_number}<br>
                            Статус: ${car.status === 'available' ? 'Доступен' : car.status === 'rented' ? 'Арендован' : car.status === 'maintenance' ? 'На обслуживании' : car.status === 'pending_completion' ? 'Ожидает завершения' : car.status}<br>
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
            const errorData = await response.json();
            console.error('Ошибка при получении позиций машин:', errorData.detail || 'Неизвестная ошибка');
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
    // Проверяем наличие валидного токена
    if (isAuthenticated()) {
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
    
    // Проверяем статус пользователя
        if (userData.status === 'banned') {
            // Показываем только информацию о бане и кнопку выхода
            userInfo.style.display = 'flex';
            authButtons.style.display = 'none';
            logoutButton.style.display = 'block';
            adminPanelButton.style.display = 'none';
            
            document.getElementById('user-name').textContent = '';
            document.getElementById('user-cashback').textContent = '';
            
            // Удаляем обработчик клика, чтобы нельзя было перейти в профиль
            userInfo.removeEventListener('click', handleProfileClick);
            
            // Показываем сообщение о бане
            const banMessage = document.createElement('div');
            banMessage.id = 'ban-message';
            banMessage.textContent = 'Ваш аккаунт заблокирован';
            banMessage.style.cssText = `
                color: #dc3545;
                font-weight: bold;
                margin-top: 5px;
                text-align: center;
                width: 100%;
            `;
            
            // Добавляем сообщение о бане в userInfo
            userInfo.appendChild(banMessage);
            
            // Удаляем панель активной аренды, если она есть
            const rentalPanel = document.getElementById('active-rental-panel');
            if (rentalPanel) {
                rentalPanel.remove();
            }
        } else {
            // Показываем обычную информацию о пользователе
            userInfo.style.display = 'flex';
            authButtons.style.display = 'none';
            logoutButton.style.display = 'block';
            
            document.getElementById('user-name').textContent = `${userData.name} ${userData.surname}`;
            document.getElementById('user-cashback').textContent = `Кэшбэк: ${userData.cashback} BYN`;
            
            // Удаляем сообщение о бане, если оно было
            const banMessage = document.getElementById('ban-message');
            if (banMessage) {
                banMessage.remove();
            }
            
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
                    const response = await authenticatedFetch(`/users/${userData.id}`);

                    if (response.ok) {
                        const fullUserData = await response.json();
                        isAdmin = fullUserData.role_id === 1;
                    } else {
                        // Обработка ошибки получения информации о пользователе
                        const errorData = await response.json();
                        console.error('Ошибка при проверке роли пользователя:', errorData.detail || 'Неизвестная ошибка');
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
            userInfo.removeEventListener('click', handleProfileClick); // Удаляем старый обработчик, если он есть
            userInfo.addEventListener('click', handleProfileClick);
            
            // Проверяем и показываем активную аренду при входе
            setTimeout(checkAndShowActiveRental, 300); // Используем небольшой таймаут для правильного отображения
        }
}

// Функция-обработчик для перехода в профиль
function handleProfileClick(e) {
    // Проверяем, что клик не был по кнопке "Выйти"
    if (!e.target.classList.contains('logout-btn')) {
        window.location.href = '/profile';
    }
}

// Функция для загрузки информации о пользователе
async function loadUserInfo() {
    if (!isAuthenticated()) {
        showAuthButtons();
        return;
    }

    try {
        const response = await authenticatedFetch('/users/profile');

        if (response.ok) {
            const userData = await response.json();

            // Сохраняем данные пользователя
            setUserData(userData);

            // Проверяем статус пользователя
            if (userData.status === 'banned') {
                // Если пользователь заблокирован, показываем соответствующее меню
                showUserInfo(userData);

                // Также удаляем активную аренду, если она есть
                const rentalPanel = document.getElementById('active-rental-panel');
                if (rentalPanel) {
                    rentalPanel.remove();
                }
            } else {
                // Если пользователь не заблокирован, показываем обычное меню
                showUserInfo(userData);

                // Запускаем обновление карты
                startMapUpdatePolling();

                // Проверяем и показываем активную аренду после загрузки данных
                setTimeout(checkAndShowActiveRental, 100);
            }
        } else {
            // 401 или другая ошибка - просто показываем кнопки входа
            showAuthButtons();
        }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
        showAuthButtons();
    }
}

// Функция для обновления статуса авторизации
function updateAuthStatus(isAuthenticated, userData = null) {
    if (isAuthenticated && userData) {
        // Сохраняем данные пользователя
        setUserData(userData);
        // Очищаем сохраненные данные форм при успешной аутентификации
        localStorage.removeItem('loginFormData');
        localStorage.removeItem('registerFormData');
        showUserInfo(userData);
        // Проверяем и показываем активную аренду при входе
        setTimeout(checkAndShowActiveRental, 500);
    } else {
        removeAuthToken();
        removeUserData();
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
const loginBtn = document.querySelector('.login-btn');
if (loginBtn) {
    loginBtn.addEventListener('click', () => {
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
                const result = await login(email, password);

                if (result.success) {
                    updateAuthStatus(true, result.user);

                    // После успешного входа обновляем карту с машинами
                    // Очищаем сохраненные данные формы
                    localStorage.removeItem('loginFormData');

                    // Закрываем модальное окно входа
                    document.body.removeChild(modal);

                    // Проверяем и обновляем статус авторизации
                    checkAuthStatus();

                    showCarsOnMap();
                } else {
                    alert(`Ошибка входа: ${result.error}`);
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
}

const registerBtn = document.querySelector('.register-btn');
if (registerBtn) {
    registerBtn.addEventListener('click', () => {
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
                const response = await authenticatedFetch('/users/register', {
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
                            } else if (typeof error.msg === 'object' && error.msg !== null) {
                                // Проверяем, есть ли у объекта msg свойство message
                                if (error.msg.message) {
                                    return error.msg.message;
                                } else {
                                    // Если объект сложный, возвращаем его строковое представление
                                    // Проверяем наличие других полей с сообщениями об ошибке
                                    if (error.msg && typeof error.msg === 'object') {
                                        if (error.msg.message) {
                                            return error.msg.message;
                                        } else if (error.msg.msg) {
                                            return error.msg.msg;
                                        } else if (error.msg.detail) {
                                            return error.msg.detail;
                                        } else {
                                            // Проверяем, является ли объект простым объектом сообщением
                                            const simpleMsg = getSimpleMessage(error.msg);
                                            return simpleMsg !== null ? simpleMsg : JSON.stringify(error.msg);
                                        }
                                    } else {
                                        // Проверяем, является ли объект простым объектом сообщением
                                        const simpleMsg = getSimpleMessage(error.msg);
                                        return simpleMsg !== null ? simpleMsg : JSON.stringify(error.msg);
                                    }
                                }
                            } else {
                                return String(error.msg);
                            }
                        }).join(', ');
                        alert(`Ошибка регистрации: ${validationErrors}`);
                    } else if (typeof errorData.detail === 'object' && errorData.detail !== null) {
                        // Обработка ошибки, когда detail является объектом
                        if (errorData.detail.message) {
                            alert(`Ошибка регистрации: ${errorData.detail.message}`);
                        } else {
                            if (errorData.detail && typeof errorData.detail === 'object') {
                                // Проверяем, есть ли у объекта detail свойство message
                                if (errorData.detail.message) {
                                    alert(`Ошибка регистрации: ${errorData.detail.message}`);
                                } else {
                                    // Проверяем, является ли объект простым объектом с сообщением
                                    const simpleMsg = getSimpleMessage(errorData.detail);
                                    alert(`Ошибка регистрации: ${simpleMsg !== null ? simpleMsg : JSON.stringify(errorData.detail)}`);
                                }
                            } else {
                                alert(`Ошибка регистрации: ${errorData.detail || 'Неизвестная ошибка'}`);
                            }
                        }
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
}

const logoutBtn = document.querySelector('.logout-btn');
if (logoutBtn) {
    logoutBtn.addEventListener('click', async () => {
    // Логика для выхода
    console.log('Кнопка выхода нажата');

    // Вызываем API endpoint для логирования выхода
    await logout();

    // Обновление статуса авторизации
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
}

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
    // checkAndShowActiveRental() вызывается внутри loadUserInfo() после загрузки данных

    // Добавляем обработчик для кнопки "Мое местоположение"
    const locateBtn = document.getElementById('locate-user-btn');
    if (locateBtn) {
        locateBtn.addEventListener('click', goToUserLocation);
    }
});

// Функция для аренды автомобиля
async function rentCar(carId) {
    if (!isAuthenticated()) {
        alert('Для аренды автомобиля необходимо авторизоваться');
        return;
    }

    const userData = getUserData();
    if (!userData) {
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
        // Отправляем запрос на создание аренды с JWT токеном
        const response = await authenticatedFetch('/rentals/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                user_id: userData.id,
                car_id: parseInt(carId),
                started_at: new Date().toISOString(),
                price: 1,
                status: "active"
            })
        });

        if (response.ok) {
            const rentalData = await response.json();
            alert(`Автомобиль успешно арендован! Номер аренды: ${rentalData.id}`);

            // Обновляем статус машины на "rented" визуально на карте
            updateCarMarkerStatus(carId, "rented");

            // Показываем панель активной аренды сразу после аренды
            await showActiveRentalPanel(rentalData);

            // Обновляем карту, чтобы отобразить только арендованную машину
            showCarsOnMap();
            
            // Отправляем уведомление другим вкладкам
            notifyRentalChange('rental_started', rentalData);
            
            // Запускаем polling для отслеживания статуса аренды
            startActiveRentalPolling();
        } else {
            const errorData = await response.json();
            console.error('Ошибка аренды:', errorData);
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
    if (!isAuthenticated()) {
        return null;
    }

    try {
        // Запрашиваем активную аренду напрямую
        const response = await authenticatedFetch(`/rentals/active`, {
            method: 'GET',
            headers: {
                'Content-Type': 'application/json'
            }
        });

        if (response.ok) {
            const activeRental = await response.json();
            return activeRental || null;
        } else {
            const errorData = await response.json();
            console.error('Ошибка при получении аренды:', errorData.detail || 'Неизвестная ошибка');
            return null;
        }
    } catch (error) {
        console.error('Ошибка при запросе аренды:', error);
        return null;
    }
}

// Глобальный интервал для обновления активной аренды
let activeRentalPollingInterval = null;

// Глобальный интервал для обновления карты
let mapUpdateInterval = null;

// BroadcastChannel для синхронизации между вкладками
const rentalChannel = new BroadcastChannel('rental_updates');

// Отправка уведомления другим вкладкам
function notifyRentalChange(action, data = null) {
    rentalChannel.postMessage({ action, data, timestamp: Date.now() });
}

// Обработка уведомлений от других вкладок
rentalChannel.onmessage = (event) => {
    const { action, data } = event.data;
    console.log('Получено уведомление от другой вкладки:', action, data);
    
    if (action === 'rental_started') {
        // Если аренда началась, запускаем polling и обновляем карту
        if (typeof map !== 'undefined' && map) {
            showCarsOnMap();
        }
        startActiveRentalPolling();
        setTimeout(checkAndShowActiveRental, 100);
    }
    
    if (action === 'rental_ended') {
        // Если аренда завершилась:
        // 1. Останавливаем polling
        stopActiveRentalPolling();
        
        // 2. Удаляем панель активной аренды
        const panel = document.getElementById('active-rental-panel');
        if (panel) {
            panel.remove();
        }
        
        // 3. Обновляем карту (машины станут зелёными)
        if (typeof map !== 'undefined' && map) {
            showCarsOnMap();
        }
        
        // 4. Обновляем данные пользователя (кэшбэк)
        refreshUserData();
    }
};

// Функция для запуска обновления карты
function startMapUpdatePolling() {
    if (mapUpdateInterval) {
        clearInterval(mapUpdateInterval);
    }
    
    // Обновляем карту каждые 5 секунд
    mapUpdateInterval = setInterval(() => {
        // Проверяем, что карта инициализирована
        if (typeof map !== 'undefined' && map) {
            showCarsOnMap();
        }
    }, 5000);
}

// Функция для остановки обновления карты
function stopMapUpdatePolling() {
    if (mapUpdateInterval) {
        clearInterval(mapUpdateInterval);
        mapUpdateInterval = null;
    }
}

// Функция для обновления данных пользователя (кэшбэк и т.д.)
async function refreshUserData() {
    if (!isAuthenticated()) {
        return;
    }
    
    try {
        const response = await authenticatedFetch('/users/profile');
        if (response.ok) {
            const userData = await response.json();
            
            // Сохраняем обновлённые данные
            setUserData(userData);
            
            // Обновляем кэшбэк в шапке
            const cashbackElement = document.getElementById('user-cashback');
            if (cashbackElement) {
                cashbackElement.textContent = `Кэшбэк: ${userData.cashback} BYN`;
            }
            
            // Обновляем кэшбэк в профиле если есть
            const profileCashbackElement = document.getElementById('profile-cashback');
            if (profileCashbackElement) {
                profileCashbackElement.textContent = `${userData.cashback} BYN`;
            }
            
            console.log('Данные пользователя обновлены:', userData);
        }
    } catch (error) {
        console.error('Ошибка при обновлении данных пользователя:', error);
    }
}

// Функция для запуска polling активной аренды
function startActiveRentalPolling() {
    // Останавливаем предыдущий polling если есть
    if (activeRentalPollingInterval) {
        clearInterval(activeRentalPollingInterval);
    }
    
    // Проверяем каждые 3 секунды
    activeRentalPollingInterval = setInterval(async () => {
        const activeRental = await getActiveRental();
        
        // Если аренда есть, обновляем панель
        if (activeRental) {
            const panel = document.getElementById('active-rental-panel');
            if (panel) {
                // Обновляем статус в панели
                const statusElement = panel.querySelector('[data-status]');
                if (statusElement) {
                    statusElement.textContent = activeRental.status === 'active' ? 'Активна' : 
                                               activeRental.status === 'completed' ? 'Завершена' : 
                                               activeRental.status === 'cancelled' ? 'Отменена' : 
                                               activeRental.status === 'pending_completion' ? 'Ожидает завершения' : activeRental.status;
                }
            }
            // Обновляем карту для отображения изменений статуса машины
            showCarsOnMap();
        } else {
            // Если аренды нет, удаляем панель и обновляем карту
            const panel = document.getElementById('active-rental-panel');
            if (panel) {
                panel.remove();
            }
            // Обновляем карту - машина должна стать зелёной
            showCarsOnMap();
            // Останавливаем polling
            stopActiveRentalPolling();
        }
    }, 3000);
}

// Функция для остановки polling активной аренды
function stopActiveRentalPolling() {
    if (activeRentalPollingInterval) {
        clearInterval(activeRentalPollingInterval);
        activeRentalPollingInterval = null;
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
        if (!isAuthenticated()) {
            console.error('showActiveRentalPanel: пользователь не авторизован');
            return;
        }
        
        let userData = getUserData();
        
        // Если userData нет, загружаем его из профиля
        if (!userData || !userData.id) {
            try {
                const response = await authenticatedFetch('/users/profile');
                if (response.ok) {
                    userData = await response.json();
                    setUserData(userData);
                    console.log('showActiveRentalPanel: userData загружен из профиля:', userData);
                }
            } catch (error) {
                console.error('showActiveRentalPanel: ошибка загрузки userData:', error);
                return;
            }
        }
        
        if (!userData || !userData.id) {
            console.error('showActiveRentalPanel: userData или userData.id отсутствует после загрузки');
            return;
        }
        
        let rentalWithCarInfo = rental;

        // Запрашиваем расширенную информацию об аренде с информацией о машине
        try {
            const response = await authenticatedFetch(`/rentals/user/${userData.id}/with-car-info?limit=100`, {
                method: 'GET'
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
        // Получаем время начала аренды в миллисекундах с начала эпохи Unix
        const startedAtMs = new Date(rental.started_at).getTime();
        
        // Получаем текущее время в миллисекундах с начала эпохи Unix
        const nowMs = Date.now();
        
        // Рассчитываем разницу в миллисекундах (разница во времени не зависит от часового пояса)
        let timeDiff = nowMs - startedAtMs;
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
                <div>
                    <p><strong>Модель:</strong> ${rentalWithCarInfo.model || rental.car_id}</p>
                    <p><strong>Номер:</strong> ${rentalWithCarInfo.plate_number || 'Неизвестен'}</p>
                    <p><strong>Статус:</strong> ${rental.status === 'active' ? 'Активна' : rental.status === 'completed' ? 'Завершена' : rental.status === 'cancelled' ? 'Отменена' : rental.status === 'pending_completion' ? 'Ожидает завершения' : rental.status}</p>
                </div>
                <p><strong>Начало:</strong> ${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</p>
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
                // rental.started_at приходит из API в формате ISO (в UTC)
                // Получаем время начала аренды в миллисекундах с начала эпохи Unix
                const updatedStartedAtMs = new Date(rental.started_at).getTime();
                
                // Получаем текущее время в миллисекундах с начала эпохи Unix
                const updatedNowMs = Date.now();
                
                // Рассчитываем разницу в миллисекундах (разница во времени не зависит от часового пояса)
                let updatedTimeDiff = updatedNowMs - updatedStartedAtMs;
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
    if (!isAuthenticated()) {
        alert('Необходима авторизация для управления арендой');
        return;
    }

    const userData = getUserData();
    if (!userData) {
        alert('Необходима авторизация для управления арендой');
        return;
    }

    if (!confirm('Вы уверены, что хотите завершить аренду?')) {
        return;
    }

    try {
        // Получаем текущую аренду для расчета цены
        const rentalResponse = await authenticatedFetch(`/rentals/${rentalId}`, {
            method: 'GET'
        });
        
        if (!rentalResponse.ok) {
            const errorData = await rentalResponse.json();
            throw new Error(`Не удалось получить информацию об аренде: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
        
        const rental = await rentalResponse.json();
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Используем время в UTC+3 для вычислений, как указано в требованиях
        
        // Преобразуем started_at из UTC в мс с начала эпохи Unix (учитывая, что это время в UTC)
        const startedAtUTC = new Date(rental.started_at).getTime();
        
        // Получаем время окончания аренды в UTC+3
        // Сначала получаем текущее время в UTC
        const endedAt = new Date();
        // Затем конвертируем в UTC+3 (добавляем 3 часа в мс)
        const endedAtUTC3 = endedAt.getTime() + (3 * 60 * 1000);
        
        // Рассчитываем разницу в миллисекундах между временем окончания (в UTC+3) и началом аренды (в UTC, конвертированное в ту же систему отсчета)
        // Поскольку startedAt - это время в UTC, а нам нужно сравнить с временем окончания в UTC+3,
        // мы должны привести оба времени к одному часовому поясу
        // Преобразуем startedAt из UTC в UTC+3 (добавляем 3 часа в мс)
        const startedAtUTC3 = startedAtUTC + (3 * 60 * 1000);
        
        let timeDiff = endedAtUTC3 - startedAtUTC3;
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
        
        // Создаем запись в логе оплаты
        const paymentLog = {
            rental_id: rentalId,
            user_id: parseInt(userData.id),
            pay_type: 'card',  // По умолчанию оплата картой в этой функции
            card_number: null, // В этой функции не используется карта, но поле должно быть
            price: price
        };

        const paymentLogResponse = await authenticatedFetch('/payment_logs/', {
            method: 'POST',
            body: JSON.stringify(paymentLog)
        });

        if (!paymentLogResponse.ok) {
            const errorData = await paymentLogResponse.json();
            throw new Error(`Ошибка при создании записи об оплате: ${errorData.detail || 'Неизвестная ошибка'}`);
        }

        const response = await authenticatedFetch(`/rentals/${rentalId}`, {
            method: 'PUT',
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
            // Останавливаем polling активной аренды
            stopActiveRentalPolling();
            
            // Обновляем кэшбэк в UI
            const cashbackElement = document.getElementById('user-cashback');
            if (cashbackElement) {
                const userData = getUserData();
                if (userData) {
                    userData.cashback = (userData.cashback || 0) + (totalPrice * 0.03);
                    cashbackElement.textContent = `Кэшбэк: ${userData.cashback.toFixed(2)} BYN`;
                }
            }
            
            alert(`Аренда успешно завершена! С вас списано: ${price} BYN (1 BYN за начало + ${minutesDiff * 0.5} BYN за ${minutesDiff} минут).`);
            
            // Отправляем уведомление другим вкладкам
            notifyRentalChange('rental_ended', { rentalId: rentalId, carId: updatedRental.car_id });
            
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
async function showCompletionModal(rentalId) {
    // Проверяем, существует ли уже модальное окно
    const existingModal = document.getElementById('completion-modal');
    if (existingModal) {
        existingModal.remove();
    }

    // Получаем информацию о текущей аренде для расчета цены
    if (!isAuthenticated()) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }

    const userData = getUserData();
    if (!userData) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }

    // Асинхронно получаем информацию об аренде и рассчитываем цену
    authenticatedFetch(`/rentals/${rentalId}`, {
        method: 'GET'
    })
    .then(response => response.json())
    .then(rental => {
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Используем время в UTC+3 для вычислений, как указано в требованиях
        
        // Преобразуем started_at из UTC в мс с начала эпохи Unix (учитывая, что это время в UTC)
        const startedAtUTC = new Date(rental.started_at).getTime();
        
        // Получаем текущее время в UTC+3
        // Сначала получаем текущее время в UTC
        const now = new Date();
        // Затем конвертируем в UTC+3 (добавляем 3 часа в мс)
        const nowUTC3 = now.getTime() + (3 * 60 * 1000);
        
        // Рассчитываем разницу в миллисекундах между текущим временем (в UTC+3) и началом аренды (в UTC, конвертированное в ту же систему отсчета)
        // Поскольку startedAt - это время в UTC, а нам нужно сравнить с текущим временем в UTC+3,
        // мы должны привести оба времени к одному часовому поясу
        // Преобразуем startedAt из UTC в UTC+3 (добавляем 3 часа в мс)
        const startedAtUTC3 = startedAtUTC + (3 * 60 * 1000);
        
        let timeDiff = nowUTC3 - startedAtUTC3;
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
                    <p><strong>Начало аренды:</strong> ${new Date(rental.started_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</p>
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
                        <div style="margin-top: 5px; font-size: 0.8em; color: #66;">Максимум можно использовать: <span id="max-cashback-amount">0</span> BYN</div>
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
    if (!isAuthenticated()) {
        return;
    }

    try {
        const response = await authenticatedFetch('/users/profile', {
            method: 'GET'
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
    if (!isAuthenticated()) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }

    let userData = getUserData();
    if (!userData) {
        alert('Необходима авторизация для завершения аренды');
        return;
    }

    try {
        const rentalResponse = await authenticatedFetch(`/rentals/${rentalId}`, {
            method: 'GET'
        });
        
        if (!rentalResponse.ok) {
            throw new Error('Не удалось получить информацию об аренде');
        }
        
        const rental = await rentalResponse.json();
        
        // rental.started_at приходит из API в формате ISO (в UTC)
        // Для корректного вычисления разницы, оба времени должны быть в одинаковой таймзоне
        // Используем время в UTC+3 для вычислений, как указано в требованиях
        
        // Преобразуем started_at из UTC в мс с начала эпохи Unix (учитывая, что это время в UTC)
        const startedAtUTC = new Date(rental.started_at).getTime();
        
        // Получаем время окончания аренды в UTC+3
        // Сначала получаем текущее время в UTC
        const endedAt = new Date();
        // Затем конвертируем в UTC+3 (добавляем 3 часа в мс)
        const endedAtUTC3 = endedAt.getTime() + (3 * 60 * 1000);
        
        // Рассчитываем разницу в миллисекундах между временем окончания (в UTC+3) и началом аренды (в UTC, конвертированное в ту же систему отсчета)
        // Поскольку startedAt - это время в UTC, а нам нужно сравнить с временем окончания в UTC+3,
        // мы должны привести оба времени к одному часовому поясу
        // Преобразуем startedAt из UTC в UTC+3 (добавляем 3 часа в мс)
        const startedAtUTC3 = startedAtUTC + (3 * 60 * 1000);
        
        let timeDiff = endedAtUTC3 - startedAtUTC3;
        if (timeDiff < 0) {
            // Если время начала аренды в будущем (из-за расхождения времени), устанавливаем разницу в 0
            timeDiff = 0;
        }
        
        const minutesDiff = Math.floor(timeDiff / (1000 * 60)); // Округляем вниз, чтобы избежать мгновенного округления вверх
        
        // Цена = 1 BYN за начало + 0.5 BYN за минуту
        const totalPrice = 1 + minutesDiff * 0.5;
        
        // Check that price is not negative or zero
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
            let userData = await getUserData();
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
        // Если использовался кэшбэк, создаем отдельные записи для кэшбэка и для оплаты картой
        
        // Проверяем, что userData.id существует
        if (!userData || !userData.id) {
            // Загружаем данные пользователя из профиля
            try {
                const response = await authenticatedFetch('/users/profile');
                if (response.ok) {
                    userData = await response.json();
                    setUserData(userData);
                }
            } catch (error) {
                console.error('Ошибка загрузки данных пользователя:', error);
                throw new Error('Не удалось загрузить данные пользователя для создания payment log');
            }
        }
        
        if (!userData || !userData.id) {
            throw new Error('userData.id отсутствует');
        }
        
        if (useCashback && cashbackUsed > 0) {
            // Создаем запись для использованного кэшбэка
            const cashbackPaymentLog = {
                rental_id: rentalId,
                user_id: parseInt(userData.id),
                user_email: userData.email || null,
                pay_type: 'cashback',
                card_number: null,
                price: cashbackUsed
            };

            // Отправляем данные оплаты кэшбэком
            const cashbackPaymentLogResponse = await authenticatedFetch('/payment_logs/', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(cashbackPaymentLog)
            });

            if (!cashbackPaymentLogResponse.ok) {
                const errorData = await cashbackPaymentLogResponse.json();
                console.error('Ошибка создания payment log (cashback):', errorData);
                throw new Error('Ошибка при создании записи об оплате кэшбэком');
            }
        }

        // Если осталась цена для оплаты картой (не вся оплата была кэшбэком)
        if (finalPrice > 0) {
            const cardPaymentLog = {
                rental_id: rentalId,
                user_id: parseInt(userData.id),
                user_email: userData.email || null,
                pay_type: 'card',
                card_number: cardNumber || null,
                price: finalPrice
            };

            console.log('Отправляем payment log:', cardPaymentLog);

            // Отправляем данные оплаты картой
            const cardPaymentLogResponse = await authenticatedFetch('/payment_logs/', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(cardPaymentLog)
            });

            if (!cardPaymentLogResponse.ok) {
                const errorData = await cardPaymentLogResponse.json();
                console.error('Ошибка создания payment log (card):', JSON.stringify(errorData, null, 2));
                const detailMsg = Array.isArray(errorData.detail) 
                    ? errorData.detail.map(d => d.msg || d.msg?.message || JSON.stringify(d)).join(', ')
                    : errorData.detail;
                throw new Error(`Ошибка при создании записи об оплате картой: ${detailMsg || 'Неизвестная ошибка'}`);
            }
        }

        // Если вся оплата была кэшбэком (finalPrice = 0), создаем только запись о кэшбэке
        else if (finalPrice === 0 && useCashback && cashbackUsed > 0) {
            // Запись уже создана выше, ничего дополнительно не нужно
        }


        // Проверяем, загружены ли фотографии (минимум 1 обязательно)
        const photoUpload = document.getElementById('photo-upload');
        const files = photoUpload.files;

        if (files.length === 0) {
            alert('Пожалуйста, загрузите хотя бы одну фотографию завершения поездки.');
            return;
        }

        // Загружаем все фотографии и создаем массив ID фотографий
        const completionPhotoIds = [];
        for (let i = 0; i < files.length; i++) {
            const file = files[i];
            if (file.type.startsWith('image/')) {
                const photoId = await uploadPhoto(file, rental.car_id, parseInt(userData.id));
                if (photoId) {
                    completionPhotoIds.push(photoId);
                }
            }
        }

        if (completionPhotoIds.length === 0) {
            alert('Не удалось загрузить фотографии. Пожалуйста, попробуйте снова.');
            return;
        }
        
        // Сначала создаем запрос на подтверждение завершения поездки (для сохранения документа)
        // Аренда еще в статусе active, чтобы пройти проверку в бэкенде
        const tripCompletionResponse = await authenticatedFetch('/trip-completions/', {
            method: 'POST',
            body: JSON.stringify({
                rental_id: rentalId,
                completion_photo_ids: completionPhotoIds, // Send array of photo IDs instead of single ID
                admin_approved: null // Set to null initially, to be reviewed by admin
            })
        });

        if (tripCompletionResponse.ok) {
            // После успешного создания запроса на завершение, обновляем статус аренды
            // Update the rental to set end date and status to pending_completion
            // Use the properly calculated time in UTC+3
            const updateRentalResponse = await authenticatedFetch(`/rentals/${rentalId}`, {
                method: 'PUT',
                body: JSON.stringify({
                    ended_at: endedAt.toISOString(),
                    status: "pending_completion", // Changed from "completed" to "pending_completion"
                    price: totalPrice  // Update the price with the calculated total
                })
            });
            
            if (!updateRentalResponse.ok) {
                throw new Error('Ошибка при обновлении статуса аренды');
            }
            
            const updatedRental = await updateRentalResponse.json();
        }
        
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

            alert(`Аренда успешно завершена! С вас списано: ${finalPrice} BYN (1 BYN за начало + ${minutesDiff * 0.5} BYN за ${minutesDiff} минут). Добавлено кэшбэка: ${cashbackToAdd.toFixed(2)} BYN.`);

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
        
        // Закрываем модальное окно в случае ошибки
        const modal = document.getElementById('completion-modal');
        if (modal) {
            modal.remove();
        }
    }
}

// Функция для загрузки фотографии
async function uploadPhoto(file, carId, userId) {
    if (!isAuthenticated()) {
        console.error('Необходима авторизация для загрузки фотографий');
        return null;
    }

    const userData = getUserData();
    if (!userData) {
        console.error('Необходима авторизация для загрузки фотографий');
        return null;
    }

    try {
        // Создаем FormData для отправки файла
        const formData = new FormData();
        formData.append('file', file);
        formData.append('object_type', 'car');
        formData.append('car_id', carId);
        formData.append('user_id', userData.id);
        formData.append('uploaded_by', userData.id);

        // Отправляем запрос на загрузку фотографии
        const response = await authenticatedFetch('/photos/upload', {
            method: 'POST',
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
    if (!isAuthenticated()) {
        return null;
    }

    try {
        const response = await authenticatedFetch('/users/profile', {
            method: 'GET'
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
    if (!isAuthenticated()) {
        return;
    }

    try {
        const userData = await getUserData();
        if (!userData) {
            return;
        }

        const newCashback = userData.cashback + amount;

        // Обновляем данные пользователя
        const response = await authenticatedFetch(`/users/${userData.id}`, {
            method: 'PUT',
            body: JSON.stringify({
                cashback: newCashback
            })
        });

        if (!response.ok) {
            const errorData = await response.json();
            console.error('Ошибка при обновлении кэшбэка:', errorData.detail || 'Неизвестная ошибка');
        }
    } catch (error) {
        console.error('Ошибка при обновлении кэшбэка:', error);
    }
}

// Function to get user IP has been removed as per requirements
 

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

// Функция для перехода на админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки и отображения активной аренды при загрузке
async function checkAndShowActiveRental() {
    const activeRental = await getActiveRental();
    if (activeRental) {
        await showActiveRentalPanel(activeRental);
        // Запускаем polling для обновления статуса аренды
        startActiveRentalPolling();
    }
}