// Функция для возврата на предыдущую страницу
function goBack() {
    window.history.back();
}

// Загрузка информации о пользователе при загрузке страницы профиля
document.addEventListener('DOMContentLoaded', async () => {
    await loadProfileInfo();
});

// Функция для загрузки информации о пользователе
async function loadProfileInfo() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        // Если пользователь не авторизован, перенаправляем на главную страницу
        window.location.href = '/';
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
            
            // Заполняем информацию о пользователе
            document.getElementById('profile-name').textContent = userData.name;
            document.getElementById('profile-surname').textContent = userData.surname;
            document.getElementById('profile-email').textContent = userData.email;
            document.getElementById('profile-cashback').textContent = `${userData.cashback} BYN`;
            
            // Получаем информацию о роли пользователя из базы данных
            const roleResponse = await fetch(`/roles/${userData.role_id}`);
            if (roleResponse.ok) {
                const roleData = await roleResponse.json();
                document.getElementById('profile-role').textContent = roleData.name;
            } else {
                // В случае ошибки используем резервный вариант
                const roleName = userData.role_id === 1 ? 'Администратор' :
                               userData.role_id === 2 ? 'Пользователь' :
                               userData.role_id === 3 ? 'Менеджер' : 'Неизвестная роль';
                document.getElementById('profile-role').textContent = roleName;
            }
            
            // Обновляем токен, если он был возвращен с сервера
            if (userData.token) {
                localStorage.setItem('auth_token', userData.token);
            }
        } else {
            // Если токен недействителен, удаляем его и перенаправляем на главную страницу
            localStorage.removeItem('auth_token');
            window.location.href = '/';
        }
    } catch (error) {
        console.error('Ошибка при загрузке информации о пользователе:', error);
        // При ошибке перенаправляем на главную страницу
        window.location.href = '/';
    }
}

// Обработчик для кнопки "Редактировать профиль"
document.querySelector('.edit-profile-btn').addEventListener('click', () => {
    openEditProfileModal();
});

// Обработчик для кнопки "Сменить пароль"
document.querySelector('.change-password-btn').addEventListener('click', () => {
    openChangePasswordModal();
});

// Функция открытия модального окна смены пароля
function openChangePasswordModal() {
    // Очищаем форму
    document.getElementById('change-password-form').reset();
    
    // Показываем модальное окно
    document.getElementById('change-password-modal').style.display = 'block';
}

// Закрытие модального окна смены пароля при клике на крестик
document.querySelector('.close-change-password').addEventListener('click', () => {
    document.getElementById('change-password-modal').style.display = 'none';
});

// Закрытие модального окна смены пароля при клике на кнопку "Отмена"
document.querySelector('.cancel-change-password').addEventListener('click', () => {
    document.getElementById('change-password-modal').style.display = 'none';
});

// Закрытие модального окна смены пароля при клике вне его области
window.addEventListener('click', (event) => {
    const modal = document.getElementById('change-password-modal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
});

// Обработчик отправки формы смены пароля
document.getElementById('change-password-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const currentPassword = document.getElementById('current-password').value;
    const newPassword = document.getElementById('new-password').value;
    const confirmPassword = document.getElementById('confirm-new-password').value;
    
    // Проверяем, совпадают ли новые пароли
    if (newPassword !== confirmPassword) {
        alert('Новые пароли не совпадают');
        return;
    }
    
    // Проверяем длину нового пароля
    if (newPassword.length < 1) {
        alert('Новый пароль должен содержать хотя бы 1 символ');
        return;
    }
    
    const formData = {
        current_password: currentPassword,
        new_password: newPassword
    };
    
    try {
        const response = await fetch('/users/change-password', {
            method: 'PUT',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        if (response.ok) {
            // Закрываем модальное окно
            document.getElementById('change-password-modal').style.display = 'none';
            
            alert('Пароль успешно изменен');
        } else {
            let errorMessage = 'Неизвестная ошибка';
            try {
                const errorData = await response.json();
                // Исправляем получение сообщения об ошибке
                if (errorData && typeof errorData === 'object') {
                    if (errorData.detail) {
                        errorMessage = errorData.detail;
                    } else if (errorData.message) {
                        errorMessage = errorData.message;
                    } else {
                        // Если detail и message нет, преобразуем объект в строку
                        errorMessage = JSON.stringify(errorData);
                    }
                } else {
                    errorMessage = errorData || 'Неизвестная ошибка';
                }
            } catch (e) {
                // Если не удалось распарсить JSON, используем текст ошибки
                errorMessage = await response.text() || 'Ошибка при смене пароля';
            }
            alert(`Ошибка при смене пароля: ${errorMessage}`);
        }
    } catch (error) {
        console.error('Ошибка при смене пароля:', error);
        alert('Ошибка при смене пароля');
    }
});

// Функция открытия модального окна редактирования профиля
function openEditProfileModal() {
    const modal = document.getElementById('edit-profile-modal');
    const nameField = document.getElementById('edit-name');
    const surnameField = document.getElementById('edit-surname');
    const emailField = document.getElementById('edit-email');
    
    // Заполняем поля текущими значениями
    nameField.value = document.getElementById('profile-name').textContent;
    surnameField.value = document.getElementById('profile-surname').textContent;
    emailField.value = document.getElementById('profile-email').textContent;
    
    // Показываем модальное окно
    modal.style.display = 'block';
}

// Закрытие модального окна при клике на крестик
document.querySelector('.close').addEventListener('click', () => {
    document.getElementById('edit-profile-modal').style.display = 'none';
});

// Закрытие модального окна при клике на кнопку "Отмена"
document.querySelector('.cancel-edit').addEventListener('click', () => {
    document.getElementById('edit-profile-modal').style.display = 'none';
});

// Закрытие модального окна при клике вне его области
window.addEventListener('click', (event) => {
    const modal = document.getElementById('edit-profile-modal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
});

// Обработчик отправки формы редактирования профиля
document.getElementById('edit-profile-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    const formData = {
        name: document.getElementById('edit-name').value,
        surname: document.getElementById('edit-surname').value,
        email: document.getElementById('edit-email').value
    };
    
    try {
        const response = await fetch('/users/profile', {
            method: 'PUT',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        if (response.ok) {
            const userData = await response.json();
            
            // Обновляем информацию на странице
            document.getElementById('profile-name').textContent = userData.name;
            document.getElementById('profile-surname').textContent = userData.surname;
            document.getElementById('profile-email').textContent = userData.email;
            
            // Закрываем модальное окно
            document.getElementById('edit-profile-modal').style.display = 'none';
            
            alert('Профиль успешно обновлен');
        } else {
            const errorData = await response.json();
            alert(`Ошибка при обновлении профиля: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении профиля:', error);
        alert('Ошибка при обновлении профиля');
    }
});