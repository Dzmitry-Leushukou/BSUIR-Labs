// Функция для возврата в админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки, является ли пользователь администратором
async function checkAdminAccess() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        window.location.href = '/';
        return false;
    }
    
    try {
        const response = await fetch('/users/profile', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const userData = await response.json();
            if (userData.role_id !== 1) { // admin role ID is 1
                alert('Доступ запрещен. Только администраторы могут просматривать эту страницу.');
                window.location.href = '/';
                return false;
            }
            return true;
        } else {
            const errorData = await response.json();
            alert(`Ошибка при проверке прав доступа: ${errorData.detail || 'Неизвестная ошибка'}`);
            window.location.href = '/';
            return false;
        }
    } catch (error) {
        console.error('Ошибка при проверке прав доступа:', error);
        alert('Ошибка при проверке прав доступа');
        window.location.href = '/';
        return false;
    }
}

// Функция для отображения модального окна с фотографией
function showPhotoModal(photoUrl, title) {
    // Удаляем предыдущее модальное окно, если оно существует
    const existingModal = document.getElementById('photo-modal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // Создаем модальное окно
    const modal = document.createElement('div');
    modal.id = 'photo-modal';
    modal.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background-color: rgba(0,0,0,0.8);
        display: flex;
        justify-content: center;
        align-items: center;
        z-index: 10000;
        cursor: pointer;
    `;
    
    modal.innerHTML = `
        <div style="position: relative; max-width: 90%; max-height: 90%;">
            <img src="${photoUrl}" alt="${title}" style="max-width: 100%; max-height: 100%; display: block;">
            <span style="
                position: absolute; 
                top: -30px; 
                right: 0; 
                color: white; 
                font-size: 30px; 
                font-weight: bold; 
                cursor: pointer;
                background: #333;
                border-radius: 50%;
                width: 30px;
                height: 30px;
                display: flex;
                align-items: center;
                justify-content: center;
            " onclick="closePhotoModal()">×</span>
        </div>
    `;
    
    document.body.appendChild(modal);
}

// Функция для закрытия модального окна с фотографией
function closePhotoModal() {
    const modal = document.getElementById('photo-modal');
    if (modal) {
        modal.remove();
    }
}

// Добавляем обработчик клика на документ для закрытия модального окна
document.addEventListener('click', function(event) {
    const modal = document.getElementById('photo-modal');
    if (modal && event.target === modal) {
        closePhotoModal();
    }
});

// Выполняем проверку прав доступа при загрузке страницы
document.addEventListener('DOMContentLoaded', async function() {
    await checkAdminAccess();
});