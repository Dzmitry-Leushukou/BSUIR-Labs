// Функция для возврата в админ панель
function goToAdminPanel() {
    window.location.href = '/admin';
}

// Функция для проверки, является ли пользователь администратором
async function checkAdminAccess() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        window.location.href = '/';
        return false;
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

// Выполняем проверку прав доступа при загрузке страницы
document.addEventListener('DOMContentLoaded', async function() {
    await checkAdminAccess();
});