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
            
            // Определяем роль пользователя
            const roleName = userData.role_id === 1 ? 'Пользователь' :
                           userData.role_id === 2 ? 'Администратор' :
                           userData.role_id === 3 ? 'Менеджер' : 'Неизвестная роль';
            document.getElementById('profile-role').textContent = roleName;
            
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
    // Пока что просто выводим сообщение, в дальнейшем можно реализовать модальное окно смены пароля
    alert('Функция смены пароля будет реализована позже');
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