// Функция для возврата на главную страницу
function goToMainPage() {
    window.location.href = '/';
}

// Функция для отображения выбранной таблицы
function showTable(tableName) {
    switch(tableName) {
        case 'action_logs':
            showActionLogsTable();
            break;
        case 'cars':
            showCarsTable();
            break;
        case 'maintenance_requests':
            showMaintenanceRequestsTable();
            break;
        case 'payment_logs':
            showPaymentLogsTable();
            break;
        case 'driver_licenses':
            showDriverLicensesTable();
            break;
        case 'rentals':
            showRentalsTable();
            break;
        case 'users':
            showUsersTable();
            break;
        case 'trip_completions':
            showTripCompletionsTable();
            break;
        default:
            console.error('Неизвестная таблица:', tableName);
    }
}

// Функция для отображения таблицы Action logs
function showActionLogsTable() {
    // Пока что просто перенаправляем на страницу таблицы
    window.location.href = '/admin/action_logs';
}

// Функция для отображения таблицы Cars
function showCarsTable() {
    window.location.href = '/admin/cars';
}

// Функция для отображения таблицы Maintenance requests
function showMaintenanceRequestsTable() {
    window.location.href = '/admin/maintenance_requests';
}

// Функция для отображения таблицы Payment logs
function showPaymentLogsTable() {
    window.location.href = '/admin/payment_logs';
}

// Функция для отображения таблицы подтверждения документов
function showDriverLicensesTable() {
    window.location.href = '/admin/driver_licenses';
}

// Функция для отображения таблицы Rentals
function showRentalsTable() {
    window.location.href = '/admin/rentals';
}

// Функция для отображения таблицы Users
function showUsersTable() {
    window.location.href = '/admin/users';
}

// Функция для отображения таблицы подтверждения завершения поездок
function showTripCompletionsTable() {
    window.location.href = '/admin/trip_completions';
}