// Функция для загрузки и отображения данных таблицы Users
async function loadUsers() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем всех пользователей (с большим лимитом)
        const response = await fetch('/users/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const users = await response.json();
            displayUsers(users);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Users: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Users:', error);
        alert('Ошибка при загрузке Users');
    }
}

// Функция для отображения данных в таблице Users
function displayUsers(users) {
    const tableBody = document.getElementById('users-table-body');
    tableBody.innerHTML = '';
    
    users.forEach(user => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${user.id}</td>
            <td>${user.email}</td>
            <td>${user.name}</td>
            <td>${user.surname}</td>
            <td>${user.cashback} BYN</td>
            <td>${user.role_id}</td>
            <td class="status-${user.status}">${user.status === 'active' ? 'Активен' : user.status === 'banned' ? 'Заблокирован' : user.status}</td>
            <td>${new Date(user.created_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>${new Date(user.updated_at).toLocaleString('ru-RU', { timeZone: 'Europe/Moscow' })}</td>
            <td>
                <button class="btn action-btn role-btn" onclick="changeRole(${user.id})">Изменить роль</button>
                <button class="btn action-btn ${user.status === 'active' ? 'block-btn' : 'unblock-btn'}"
                    onclick="toggleUserStatus(${user.id}, '${user.status === 'active' ? 'banned' : 'active'}')">
                    ${user.status === 'active' ? 'Заблокировать' : 'Разблокировать'}
                </button>
                <button class="btn action-btn delete-btn" onclick="deleteUser(${user.id})">Удалить</button>
            </td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для изменения роли пользователя
async function changeRole(userId) {
    const userIdFromStorage = localStorage.getItem('user_id');
    if (!userIdFromStorage) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const newRoleId = prompt('Введите ID новой роли (1 - admin, 2 - user, 3 - manager):');
    if (!newRoleId) return;
    
    if (!confirm(`Вы уверены, что хотите изменить роль пользователя на ${newRoleId}?`)) {
        return;
    }
    
    try {
        const response = await fetch(`/users/${userId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userIdFromStorage,
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
            alert(`Ошибка при обновлении роли: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении роли:', error);
        alert('Ошибка при обновлении роли');
    }
}

// Функция для изменения статуса пользователя (блокировка/разблокировка)
async function toggleUserStatus(userId, newStatus) {
    const userIdFromStorage = localStorage.getItem('user_id');
    if (!userIdFromStorage) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const statusText = newStatus === 'active' ? 'активен' : 'заблокирован';
    if (!confirm(`Вы уверены, что хотите изменить статус пользователя на "${statusText}"?`)) {
        return;
    }
    
    try {
        const response = await fetch(`/users/${userId}`, {
            method: 'PUT',
            headers: {
                'X-User-ID': userIdFromStorage,
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
            alert(`Ошибка при обновлении статуса: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении статуса:', error);
        alert('Ошибка при обновлении статуса');
    }
}

// Функция для удаления пользователя
async function deleteUser(userId) {
    const userIdFromStorage = localStorage.getItem('user_id');
    if (!userIdFromStorage) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm('Вы уверены, что хотите удалить этого пользователя? Это действие необратимо.')) {
        return;
    }
    
    try {
        const response = await fetch(`/users/${userId}`, {
            method: 'DELETE',
            headers: {
                'X-User-ID': userIdFromStorage,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            alert('Пользователь успешно удален');
            // Перезагружаем таблицу
            loadUsers();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при удалении пользователя: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при удалении пользователя:', error);
        alert('Ошибка при удалении пользователя');
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