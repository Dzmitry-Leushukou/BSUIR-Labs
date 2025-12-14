// Функция для загрузки и отображения данных таблицы Payment logs
async function loadPaymentLogs() {
    const userId = localStorage.getItem('user_id');
    if (!userId) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        // Загружаем все логи платежей (с большим лимитом)
        const response = await fetch('/payment_logs/?offset=0&limit=10000', {
            method: 'GET',
            headers: {
                'X-User-ID': userId,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const paymentLogs = await response.json();
            // Payment logs are already sorted by backend (newest first), so just display them
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
        `;
        tableBody.appendChild(row);
    });
}

// Функция для фильтрации данных таблицы Payment logs
function filterPaymentLogs() {
    const filterInputs = document.querySelectorAll('input.filter-input');
    const rows = document.querySelectorAll('#payment-logs-table-body tr');
    
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
    loadPaymentLogs();
    
    // Устанавливаем обработчики для фильтров
    const filterInputs = document.querySelectorAll('.filter-input');
    filterInputs.forEach(input => {
        input.addEventListener('input', filterPaymentLogs);
    });
});