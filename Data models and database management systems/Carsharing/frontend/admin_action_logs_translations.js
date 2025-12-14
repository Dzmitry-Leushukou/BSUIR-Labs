// Файл с переводами для action logs
const actionTypeTranslations = {
    'user_login': 'Вход пользователя',
    'user_logout': 'Выход пользователя',
    'user_registration': 'Регистрация пользователя',
    'car_rental_start': 'Начало аренды автомобиля',
    'car_rental_end': 'Завершение аренды автомобиля',
    'car_rental_cancel': 'Отмена аренды автомобиля',
    'car_rental_pending_completion': 'Ожидание завершения аренды',
    'payment_success': 'Успешный платеж',
    'payment_failed': 'Неудачный платеж',
    'maintenance_request': 'Запрос на обслуживание',
    'maintenance_resolve': 'Решение запроса на обслуживание',
    'profile_update': 'Обновление профиля',
    'driver_license_upload': 'Загрузка водительских прав',
    'car_status_change': 'Изменение статуса автомобиля',
    'user_status_change': 'Изменение статуса пользователя',
    'car_create': 'Создание автомобиля',
    'car_update': 'Обновление автомобиля',
    'car_delete': 'Удаление автомобиля',
    'driver_license_create': 'Создание водительских прав',
    'driver_license_update': 'Обновление водительских прав',
    'driver_license_delete': 'Удаление водительских прав',
    'maintenance_request_create': 'Создание запроса на обслуживание',
    'maintenance_request_update': 'Обновление запроса на обслуживание',
    'maintenance_request_delete': 'Удаление запроса на обслуживание',
    'car_photo_upload': 'Загрузка фото автомобиля',
    'car_photo_delete': 'Удаление фото автомобиля',
    'trip_completion_create': 'Создание завершения поездки',
    'trip_completion_update': 'Обновление завершения поездки',
    'user_ban': 'Блокировка пользователя',
    'user_unban': 'Разблокировка пользователя'
};

function translateActionType(actionType) {
    return actionTypeTranslations[actionType] || actionType;
}

function translateDescription(description) {
    // Основные описания уже могут быть на русском, но если есть английские варианты, добавим их сюда
    const descriptionTranslations = {
        'User profile updated': 'Профиль пользователя обновлен',
        'New user registration': 'Новая регистрация пользователя',
        'Car rental started': 'Аренда автомобиля началась',
        'Rental status changed': 'Статус аренды изменен',
        'Payment completed successfully': 'Платеж успешно завершен'
    };
    
    return descriptionTranslations[description] || description;
}

// Экспортируем функции для использования в других скриптах
window.translateActionType = translateActionType;
window.translateDescription = translateDescription;