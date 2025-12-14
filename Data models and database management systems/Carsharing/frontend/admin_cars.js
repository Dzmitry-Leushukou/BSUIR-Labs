// Глобальные переменные для пагинации
let currentCarPage = 0;
const carsPerPage = 10;

// Функция для загрузки и отображения данных таблицы Cars с пагинацией
async function loadCars(page = 0) {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    currentCarPage = pageNum;
    const offset = pageNum * carsPerPage;
    
    try {
        // Загружаем автомобили с пагинацией
        const response = await fetch(`/cars/?offset=${offset}&limit=${carsPerPage}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const cars = await response.json();
            displayCars(cars);
            setupCarPagination(page);
        } else {
            const errorData = await response.json();
            let errorMessage = 'Неизвестная ошибка';
            if (errorData && typeof errorData === 'object') {
                if (errorData.detail) {
                    errorMessage = errorData.detail;
                } else if (errorData.message) {
                    errorMessage = errorData.message;
                } else {
                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                }
            } else {
                errorMessage = errorData || 'Неизвестная ошибка';
            }
            alert(`Ошибка при загрузке Cars: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Cars:', error);
        alert('Ошибка при загрузке Cars');
    }
}

// Функция для настройки пагинации автомобилей
function setupCarPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество автомобилей для определения количества страниц
    getCarCount().then(totalCount => {
        // Проверяем, что totalCount - валидное число
        const count = totalCount || 0;
        const totalPages = Math.ceil(count / carsPerPage);
        
        // Use the validated page number throughout the function
        const safePageNum = validPageNum;
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('cars-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'cars-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#cars-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'cars-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#cars-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#cars-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#cars-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadCars(${safePageNum - 1})">Предыдущая</button>`;
        }
        
        // Кнопки страниц
        const maxVisiblePages = 5;
        let startPage = Math.max(0, safePageNum - Math.floor(maxVisiblePages / 2));
        let endPage = Math.min(totalPages - 1, startPage + maxVisiblePages - 1);
        
        if (endPage - startPage + 1 < maxVisiblePages) {
            startPage = Math.max(0, endPage - maxVisiblePages + 1);
        }
        
        for (let i = startPage; i <= endPage; i++) {
            if (i === safePageNum) {
                paginationHTML += `<button class="pagination-btn active">${i + 1}</button>`;
            } else {
                paginationHTML += `<button class="pagination-btn" onclick="loadCars(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadCars(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    }).catch(error => {
        console.error('Ошибка при настройке пагинации:', error);
        // Создаем контейнер для пагинации даже если возникла ошибка при получении количества
        let paginationContainer = document.getElementById('cars-pagination');
        if (!paginationContainer) {
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'cars-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#cars-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'cars-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#cars-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#cars-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#cars-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        // Выводим кнопку обновления, если возникла ошибка
        paginationContainer.innerHTML = '<button class="pagination-btn" onclick="loadCars(0)">Обновить</button>';
    });
}

// Функция для получения общего количества автомобилей
async function getCarCount() {
    try {
        const userId = localStorage.getItem('user_id');
        if (!userId) {
            throw new Error('Пользователь не авторизован');
        }
        
        const response = await fetch('/cars/count', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const countData = await response.json();
            return countData.count || 0;
        } else {
            const errorData = await response.json();
            console.error('Ошибка сервера при получении количества автомобилей:', errorData);
            return 0;
        }
    } catch (error) {
        console.error('Ошибка при получении количества автомобилей:', error);
        return 0;
    }
}

// Функция для отображения данных в таблице Cars
function displayCars(cars) {
    const tableBody = document.getElementById('cars-table-body');
    tableBody.innerHTML = '';
    
    cars.forEach(car => {
        // Форматируем координаты из геометрии
        let position = '';
        if (car.position) {
            // Проверяем, является ли position строкой в формате POINT
            if (typeof car.position === 'string' && car.position.startsWith('POINT')) {
                // Пример: "POINT(27.5615 53.9041)" -> "53.9041, 27.5615"
                const match = car.position.match(/POINT\(([-+]?\d*\.\d+|\d+) ([-+]?\d*\.\d+|\d+)\)/);
                if (match) {
                    position = `${match[2]}, ${match[1]}`; // широта, долгота
                } else {
                    // Если формат не соответствует ожидаемому, выводим как есть
                    position = car.position;
                }
            } else {
                // Если position не строка в формате POINT, возможно это уже объект с координатами
                // или просто строка координат, выводим как есть
                position = car.position;
            }
        } else {
            // Если позиция отсутствует, проверим, возможно есть поля latitude и longitude
            // как в функции get_cars_positions_with_user_rental_status
            if (car.latitude !== undefined && car.longitude !== undefined) {
                position = `${car.latitude}, ${car.longitude}`;
            } else {
                position = 'Не задана';
            }
        }
        
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${car.id}</td>
            <td>${car.vin}</td>
            <td>${car.plate_number}</td>
            <td>${car.model}</td>
            <td>${car.status === 'available' ? 'Доступен' : car.status === 'rented' ? 'Арендован' : car.status === 'maintenance' ? 'На обслуживании' : car.status === 'pending_completion' ? 'Ожидает завершения' : car.status}</td>
            <td>${position}</td>
            <td>${new Date(car.updated_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>
                <button class="btn edit-btn" onclick="openEditCarModal(${car.id})">Редактировать</button>
            </td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для фильтрации данных таблицы Cars
function filterCars() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#cars-table-body tr');
    
    rows.forEach(row => {
        let shouldShow = true;
        
        // Проверяем каждую ячейку в строке
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Получаем значение ячейки в той же колонке, что и фильтр
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
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterCars);
    });
});

// Функция для открытия модального окна создания автомобиля
function openCreateCarModal() {
    document.getElementById('car-modal-title').textContent = 'Добавить автомобиль';
    document.getElementById('car-form').reset();
    document.getElementById('car-id').value = '';
    document.getElementById('car-modal').style.display = 'block';
}

// Функция для открытия модального окна редактирования автомобиля
async function openEditCarModal(carId) {
    try {
        const userId = localStorage.getItem('user_id');
        if (!userId) {
            alert('Пользователь не авторизован');
            return;
        }
        
        const response = await fetch(`/cars/${carId}`, {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const car = await response.json();
            
            // Форматируем координаты из геометрии для редактирования
            let position = '';
            if (car.position) {
                const match = car.position.match(/POINT\(([-+]?\d*\.\d+|\d+) ([-+]?\d*\.\d+|\d+)\)/);
                if (match) {
                    position = `${match[2]}, ${match[1]}`; // широта, долгота
                }
            }
            
            document.getElementById('car-modal-title').textContent = 'Редактировать автомобиль';
            document.getElementById('car-id').value = car.id;
            document.getElementById('vin').value = car.vin;
            document.getElementById('plate_number').value = car.plate_number;
            document.getElementById('model').value = car.model;
            document.getElementById('status').value = car.status;
            
            document.getElementById('car-modal').style.display = 'block';
        } else {
            const errorData = await response.json();
            let errorMessage = 'Неизвестная ошибка';
            if (errorData && typeof errorData === 'object') {
                if (errorData.detail) {
                    errorMessage = errorData.detail;
                } else if (errorData.message) {
                    errorMessage = errorData.message;
                } else {
                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                }
            } else {
                errorMessage = errorData || 'Неизвестная ошибка';
            }
            alert(`Ошибка при загрузке данных автомобиля: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке данных автомобиля:', error);
        alert('Ошибка при загрузке данных автомобиля');
    }
}

// Функция для закрытия модального окна
function closeCarModal() {
    document.getElementById('car-modal').style.display = 'none';
}

// Функция для отправки формы создания/редактирования автомобиля
async function submitCarForm(event) {
    event.preventDefault();
    
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const carId = document.getElementById('car-id').value;
    const vin = document.getElementById('vin').value;
    const plateNumber = document.getElementById('plate_number').value;
    const model = document.getElementById('model').value;
    const status = document.getElementById('status').value;
    
    const carData = {
        vin,
        plate_number: plateNumber,
        model,
        status
    };
    
    try {
        let response;
        if (carId) {
            // Режим обновления
            response = await fetch(`/cars/${carId}`, {
                method: 'PUT',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(carData)
            });
        } else {
            // Режим создания
            response = await fetch('/cars/', {
                method: 'POST',
                headers: {
                    'X-User-ID': userId,
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(carData)
            });
        }
        
        if (response.ok) {
            const result = await response.json();
            alert(`Автомобиль ${carId ? 'обновлен' : 'создан'} успешно`);
            closeCarModal();
            loadCars(); // Перезагружаем таблицу
        } else {
            const errorData = await response.json();
            let errorMessage = 'Неизвестная ошибка';
            if (errorData && typeof errorData === 'object') {
                if (errorData.detail) {
                    errorMessage = errorData.detail;
                } else if (errorData.message) {
                    errorMessage = errorData.message;
                } else {
                    errorMessage = getSimpleMessage(errorData) !== null ? getSimpleMessage(errorData) : JSON.stringify(errorData);
                }
            } else {
                errorMessage = errorData || 'Неизвестная ошибка';
            }
            alert(`Ошибка при ${carId ? 'обновлении' : 'создании'} автомобиля: ${errorMessage}`);
        }
    } catch (error) {
        console.error(`Ошибка при ${carId ? 'обновлении' : 'создании'} автомобиля:`, error);
        alert(`Ошибка при ${carId ? 'обновлении' : 'создании'} автомобиля`);
    }
}

// Добавляем обработчик события для формы
document.addEventListener('DOMContentLoaded', () => {
    loadCars();
    
    // Добавляем обработчик отправки формы
    document.getElementById('car-form').addEventListener('submit', submitCarForm);
    
    // Закрытие модального окна при клике вне его
    window.onclick = function(event) {
        const modal = document.getElementById('car-modal');
        if (event.target === modal) {
            closeCarModal();
        }
    };
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

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadCars);