// Функция для загрузки и отображения данных таблицы Payment logs
async function loadPaymentLogs() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/payment_logs/', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const paymentLogs = await response.json();
            displayPaymentLogs(paymentLogs);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Payment logs: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Payment logs:', error);
        alert('Ошибка при загрузке Payment logs');
    }
}

// Функция для отображения данных в таблице Payment logs
function displayPaymentLogs(paymentLogs) {
    const tableBody = document.getElementById('payment-logs-table-body');
    tableBody.innerHTML = '';
    
    paymentLogs.forEach(log => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${log.id}</td>
            <td>${log.rental_id}</td>
            <td>${log.user_id}</td>
            <td>${log.pay_type}</td>
            <td>${log.price} BYN</td>
            <td>${log.ip || ''}</td>
        `;
        tableBody.appendChild(row);
    });
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadPaymentLogs);