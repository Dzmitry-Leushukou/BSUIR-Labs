// Глобальные переменные для пагинации
let currentUsersPage = 0;
const usersPerPage = 10;

// Функция для загрузки и отображения данных таблицы Users с пагинацией
async function loadUsers(page = 0) {
    if (!isAuthenticated()) {
        alert('Пользователь не авторизован');
        return;
    }

    // Ensure page is a valid number
    const pageNum = parseInt(page) || 0;
    const validPageNum = isFinite(pageNum) ? pageNum : 0;
    currentUsersPage = validPageNum;
    const offset = validPageNum * usersPerPage;

    try {
        // Загружаем пользователей с пагинацией
        const response = await authenticatedFetch(`/users/?offset=${offset}&limit=${usersPerPage}`);
        
        if (response.ok) {
            const users = await response.json();
            displayUsers(users);
            setupUsersPagination(page);
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
            alert(`Ошибка при загрузке пользователей: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке пользователей:', error);
        alert('Ошибка при загрузке пользователей');
    }
}

// Функция для настройки пагинации пользователей
function setupUsersPagination(currentPage) {
    // Проверяем, что currentPage - это число
    const pageNum = parseInt(currentPage) || 0;
    // Ensure pageNum is a valid finite number
    const safePageNum = isFinite(pageNum) ? pageNum : 0;
    
    // Подсчитываем общее количество пользователей для определения количества страниц
    getUsersCount().then(totalCount => {
        const totalPages = Math.ceil(totalCount / usersPerPage);
        
        // Создаем или обновляем элемент пагинации
        let paginationContainer = document.getElementById('users-pagination');
        if (!paginationContainer) {
            // Создаем контейнер для пагинации под таблицей
            paginationContainer = document.createElement('div');
            paginationContainer.id = 'users-pagination';
            paginationContainer.className = 'pagination';
            // Проверяем, есть ли уже контейнер для таблицы, иначе создаем
            let tableContainer = document.querySelector('#users-table-container');
            if (!tableContainer) {
                tableContainer = document.createElement('div');
                tableContainer.id = 'users-table-container';
                // Перемещаем таблицу в контейнер
                const tableElement = document.querySelector('#users-table');
                if (tableElement) {
                    tableContainer.appendChild(tableElement);
                }
                // Находим родительский элемент и добавляем туда контейнер
                const tableBody = document.querySelector('#users-table-body').closest('table').parentElement;
                tableBody.parentElement.insertBefore(tableContainer, document.querySelector('#users-table-body').closest('table').parentElement.nextSibling);
            }
            tableContainer.appendChild(paginationContainer);
        }
        
        // Генерируем HTML для пагинации
        let paginationHTML = '';
        
        // Кнопка "Предыдущая"
        if (safePageNum > 0) {
            paginationHTML += `<button class="pagination-btn" onclick="loadUsers(${safePageNum - 1})">Предыдущая</button>`;
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
                paginationHTML += `<button class="pagination-btn" onclick="loadUsers(${i})">${i + 1}</button>`;
            }
        }
        
        // Кнопка "Следующая"
        if (safePageNum < totalPages - 1) {
            paginationHTML += `<button class="pagination-btn" onclick="loadUsers(${safePageNum + 1})">Следующая</button>`;
        }
        
        paginationContainer.innerHTML = paginationHTML;
    });
}

// Функция для получения общего количества пользователей
async function getUsersCount() {
    try {
        if (!isAuthenticated()) {
            throw new Error('Пользователь не авторизован');
        }

        const response = await authenticatedFetch('/users/count');
        
        if (response.ok) {
            const countData = await response.json();
            return countData.count || 0;
        } else {
            const errorData = await response.json();
            console.error('Ошибка сервера при получении количества пользователей:', errorData);
            return 0;
        }
    } catch (error) {
        console.error('Ошибка при получении количества пользователей:', error);
        return 0;
    }
}

// Функция для отображения данных в таблице Users
function displayUsers(users) {
    const tableBody = document.getElementById('users-table-body');
    tableBody.innerHTML = '';
    
    users.forEach(user => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${user.email}</td>
            <td>${user.name}</td>
            <td>${user.surname}</td>
            <td>${user.cashback} BYN</td>
            <td>${user.role_name === 'admin' ? 'Администратор' : user.role_name === 'user' ? 'Пользователь' : user.role_name}</td>
            <td class="status-${user.status}">${user.status === 'active' ? 'Активен' : user.status === 'banned' ? 'Заблокирован' : user.status}</td>
            <td>${new Date(user.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${new Date(user.updated_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>
                <button class="btn action-btn role-btn" onclick="changeRole(${user.id})">Изменить роль</button>
                <button class="btn action-btn ${user.status === 'active' ? 'block-btn' : 'unblock-btn'}"
                    onclick="toggleUserStatus(${user.id}, '${user.status === 'active' ? 'banned' : 'active'}')">
                    ${user.status === 'active' ? 'Заблокировать' : 'Разблокировать'}
                </button>
            </td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для изменения роли пользователя
async function changeRole(userId) {
    if (!isAuthenticated()) {
        alert('Пользователь не авторизован');
        return;
    }

    const newRoleId = prompt('Введите номер новой роли (1 - Администратор, 2 - Пользователь):');
    if (!newRoleId) return;

    // Проверяем, что введенный ID - это число
    const roleId = parseInt(newRoleId);
    if (isNaN(roleId) || (roleId !== 1 && roleId !== 2)) {
        alert('Неверный ID роли. Допустимые значения: 1 (Администратор) или 2 (Пользователь)');
        return;
    }

    const roleName = roleId === 1 ? 'Администратор' : 'Пользователь';
    if (!confirm(`Вы уверены, что хотите изменить роль пользователя на "${roleName}"?`)) {
        return;
    }

    try {
        const response = await authenticatedFetch(`/users/${userId}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ role_id: parseInt(newRoleId) })
        });
        
        if (response.ok) {
            alert('Роль пользователя успешно обновлена');
            // Перезагружаем таблицу
            loadUsers();
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
            alert(`Ошибка при обновлении роли: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении роли:', error);
        alert('Ошибка при обновлении роли');
    }
}

// Функция для изменения статуса пользователя (блокировка/разблокировка)
async function toggleUserStatus(userId, newStatus) {
    if (!isAuthenticated()) {
        alert('Пользователь не авторизован');
        return;
    }

    const statusText = newStatus === 'active' ? 'активен' : 'заблокирован';
    if (!confirm(`Вы уверены, что хотите изменить статус пользователя на "${statusText}"?`)) {
        return;
    }

    try {
        const response = await authenticatedFetch(`/users/${userId}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: newStatus })
        });
        
        if (response.ok) {
            alert(`Статус пользователя успешно изменен на "${statusText}"`);
            // Перезагружаем таблицу
            loadUsers();
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
            alert(`Ошибка при обновлении статуса: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении статуса:', error);
        alert('Ошибка при обновлении статуса');
    }
}


// Функция для фильтрации данных таблицы Users
function filterUsers() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#users-table-body tr');
    
    rows.forEach(row => {
        let shouldShow = true;
        
        // Проверяем каждую ячейку в строке
        for (let i = 0; i < filterInputs.length; i++) {
            const filterValue = filterInputs[i].value.trim();
            if (filterValue) {
                // Проверяем, что ячейка существует
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
    loadUsers();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterUsers);
    });
});