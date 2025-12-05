// Функция для загрузки и отображения данных таблицы Driver licenses
async function loadDriverLicenses() {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    try {
        const response = await fetch('/driver_licenses/', {
            method: 'GET',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            }
        });
        
        if (response.ok) {
            const driverLicenses = await response.json();
            displayDriverLicenses(driverLicenses);
        } else {
            const errorData = await response.json();
            alert(`Ошибка при загрузке Driver licenses: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при загрузке Driver licenses:', error);
        alert('Ошибка при загрузке Driver licenses');
    }
}

// Функция для отображения данных в таблице Driver licenses
function displayDriverLicenses(driverLicenses) {
    const tableBody = document.getElementById('driver-licenses-table-body');
    tableBody.innerHTML = '';
    
    driverLicenses.forEach(license => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${license.driver_id}</td>
            <td>${license.license_number}</td>
            <td>${license.issued_by}</td>
            <td>${new Date(license.expiration_date).toLocaleDateString('ru-RU')}</td>
            <td>${license.document_photo_id}</td>
            <td class="status-${license.status}">${license.status}</td>
            <td>
                <button class="btn action-btn approve-btn" onclick="updateLicenseStatus(${license.driver_id}, 'approved')">Подтвердить</button>
                <button class="btn action-btn reject-btn" onclick="updateLicenseStatus(${license.driver_id}, 'rejected')">Отклонить</button>
                <button class="btn action-btn skip-btn" onclick="updateLicenseStatus(${license.driver_id}, 'pending')">Пропустить</button>
            </td>
        `;
        tableBody.appendChild(row);
    });
}

// Функция для обновления статуса водительской лицензии
async function updateLicenseStatus(licenseId, status) {
    const token = localStorage.getItem('auth_token');
    if (!token) {
        alert('Пользователь не авторизован');
        return;
    }
    
    if (!confirm(`Вы уверены, что хотите изменить статус лицензии на "${status}"?`)) {
        return;
    }
    
    try {
        const response = await fetch(`/driver_licenses/${licenseId}`, {
            method: 'PUT',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ status: status })
        });
        
        if (response.ok) {
            alert(`Статус лицензии успешно обновлен на "${status}"`);
            // Перезагружаем таблицу
            loadDriverLicenses();
        } else {
            const errorData = await response.json();
            alert(`Ошибка при обновлении статуса лицензии: ${errorData.detail || 'Неизвестная ошибка'}`);
        }
    } catch (error) {
        console.error('Ошибка при обновлении статуса лицензии:', error);
        alert('Ошибка при обновлении статуса лицензии');
    }
}

// Загружаем данные при загрузке страницы
document.addEventListener('DOMContentLoaded', loadDriverLicenses);