// Файл с переводами для action logs

// Перевод типов действий
const actionTypeTranslations = {
    'user_login': 'Вход пользователя',
    'user_logout': 'Выход пользователя',
    'user_registration': 'Регистрация пользователя',
    'profile_update': 'Обновление профиля',
    'car_create': 'Создание автомобиля',
    'car_update': 'Обновление автомобиля',
    'car_delete': 'Удаление автомобиля',
    'car_status_change': 'Изменение статуса автомобиля',
    'car_rental_start': 'Начало аренды автомобиля',
    'car_rental_end': 'Завершение аренды автомобиля',
    'car_rental_cancel': 'Отмена аренды автомобиля',
    'car_rental_pending_completion': 'Ожидание завершения аренды',
    'payment_success': 'Успешный платеж',
    'payment_failed': 'Неудачный платеж',
    'maintenance_request': 'Запрос на обслуживание',
    'maintenance_request_create': 'Создание запроса на обслуживание',
    'maintenance_request_update': 'Обновление запроса на обслуживание',
    'maintenance_request_delete': 'Удаление запроса на обслуживание',
    'maintenance_resolve': 'Решение запроса на обслуживание',
    'driver_license_upload': 'Загрузка водительских прав',
    'driver_license_create': 'Создание водительских прав',
    'driver_license_update': 'Обновление водительских прав',
    'driver_license_delete': 'Удаление водительских прав',
    'driver_license_approved': 'Водительские права одобрены',
    'driver_license_rejected': 'Водительские права отклонены',
    'user_status_change': 'Изменение статуса пользователя',
    'user_ban': 'Блокировка пользователя',
    'user_unban': 'Разблокировка пользователя',
    'trip_completion_create': 'Создание завершения поездки',
    'trip_completion_approved': 'Поездка одобрена',
    'trip_completion_rejected': 'Поездка отклонена',
    'trip_completion_update': 'Обновление завершения поездки',
    'car_photo_upload': 'Загрузка фото автомобиля',
    'car_photo_delete': 'Удаление фото автомобиля'
};

function translateActionType(actionType) {
    if (!actionType) return '';
    const translation = actionTypeTranslations[actionType];
    return translation || actionType;
}

// Перевод описаний
function translateDescription(description) {
    if (!description) return '';
    
    const descriptionTranslations = {
        'Пользователь выполнил': 'Пользователь выполнил',
        'Действие:': 'Действие:',
        'Комбинированное действие:': 'Комбинированное действие:',
        'Администратор выполнил': 'Администратор выполнил',
        'Вход пользователя': 'Вход пользователя',
        'Выход пользователя': 'Выход пользователя',
        'Обновление профиля пользователя': 'Обновление профиля пользователя',
        'Новая регистрация пользователя': 'Новая регистрация пользователя',
        'Аренда автомобиля началась': 'Аренда автомобиля началась',
        'Аренда автомобиля завершена': 'Аренда автомобиля завершена',
        'Аренда автомобиля отменена': 'Аренда автомобиля отменена',
        'Статус аренды изменен': 'Статус аренды изменен',
        'Платеж успешно завершен': 'Платеж успешно завершен',
        'Платеж не выполнен': 'Платеж не выполнен',
        'Запрос на обслуживание создан': 'Запрос на обслуживание создан',
        'Запрос на обслуживание решен': 'Запрос на обслуживание решен',
        'Водительские права загружены': 'Водительские права загружены',
        'Водительские права одобрены': 'Водительские права одобрены',
        'Водительские права отклонены': 'Водительские права отклонены',
        'Пользователь заблокирован': 'Пользователь заблокирован',
        'Пользователь разблокирован': 'Пользователь разблокирован',
        'Автомобиль создан': 'Автомобиль создан',
        'Автомобиль обновлен': 'Автомобиль обновлен',
        'Автомобиль удален': 'Автомобиль удален',
        'Успешный платеж': 'Успешный платеж'
    };

    let result = description;
    
    // Сначала пробуем перевести полное описание
    if (descriptionTranslations[result]) {
        return descriptionTranslations[result];
    }
    
    // Переводим префиксы
    for (const [prefix, translation] of Object.entries(descriptionTranslations)) {
        if (result.startsWith(prefix)) {
            const rest = result.substring(prefix.length);
            // Если после префикса идет ключ действия, переводим его
            const actionTranslation = actionTypeTranslations[rest.trim()];
            if (actionTranslation) {
                return translation + ' ' + actionTranslation;
            }
            return result;
        }
    }
    
    // Если описание содержит ключ действия, переводим его
    if (actionTypeTranslations[result]) {
        return actionTypeTranslations[result];
    }
    
    return result;
}

// Перевод заголовков колонок
const columnHeaderTranslations = {
    'created_at': 'Время лога',
    'actor_email': 'Email пользователя',
    'action_type': 'Тип действия',
    'target_user_email': 'Email целевого пользователя',
    'target_car_vin': 'VIN целевого автомобиля',
    'target_rental_id': 'Номер целевой аренды',
    'description': 'Описание',
    'old_values': 'Старые значения',
    'new_values': 'Новые значения',
    'modal_title': 'Детали лога'
};

function translateColumnHeader(columnKey) {
    return columnHeaderTranslations[columnKey] || columnKey;
}

// Перевод полей в old_values/new_values
const fieldTranslations = {
    'vin': 'VIN',
    'plate_number': 'Номерной знак',
    'model': 'Модель',
    'status': 'Статус',
    'position': 'Позиция',
    'updated_at': 'Обновлено',
    'created_at': 'Создано',
    'email': 'Email',
    'name': 'Имя',
    'surname': 'Фамилия',
    'cashback': 'Кэшбэк',
    'role_id': 'Роль',
    'user_id': 'ID пользователя',
    'car_id': 'ID автомобиля',
    'rental_id': 'ID аренды',
    'description': 'Описание',
    'target_user_id': 'ID целевого пользователя',
    'target_car_id': 'ID целевого автомобиля',
    'target_rental_id': 'ID целевой аренды',
    'action_type': 'Тип действия',
    'actor_user_id': 'ID пользователя-актера',
    'old_values': 'Старые значения',
    'new_values': 'Новые значения',
    'user_agent': 'User Agent',
    'ip_address': 'IP адрес',
    'license_number': 'Номер удостоверения',
    'issued_by': 'Выдано',
    'expiration_date': 'Срок действия',
    'admin_approved': 'Админ одобрил',
    'admin_comment': 'Комментарий администратора',
    'amount': 'Сумма',
    'price': 'Цена',
    'pay_type': 'Тип платежа'
};

// Перевод статусов (enum значений)
const enumTranslations = {
    'available': 'Доступен',
    'rented': 'Арендован',
    'maintenance': 'На обслуживании',
    'out_of_service': 'Вне эксплуатации',
    'active': 'Активный',
    'banned': 'Заблокирован',
    'pending': 'Ожидает',
    'completed': 'Завершена',
    'cancelled': 'Отменена',
    'open': 'Открыт',
    'in_progress': 'В процессе',
    'resolved': 'Решен',
    'approved': 'Одобрен',
    'rejected': 'Отклонен',
    'pending_completion': 'Ожидает завершения',
    'rental_fee': 'Оплата аренды',
    'fine': 'Штраф',
    'insurance': 'Страховка',
    'success': 'Успешно',
    'failed': 'Неудачно',
    'completed': 'Завершено'
};

// Перевод ролей
const roleTranslations = {
    '1': 'Администратор',
    '2': 'Пользователь'
};

// Функция для перевода полей в JSON объектах
function translateFieldNames(jsonString) {
    try {
        const obj = JSON.parse(jsonString);

        function translateKeys(input) {
            if (Array.isArray(input)) {
                return input.map(item => translateKeys(item));
            } else if (input !== null && typeof input === 'object') {
                const translatedObj = {};
                for (const [key, value] of Object.entries(input)) {
                    if (key === 'updated_at') {
                        continue;
                    }

                    const translatedKey = fieldTranslations[key] || key;

                    if (key === 'role_id' && typeof value === 'number') {
                        translatedObj[translatedKey] = roleTranslations[value.toString()] || `Роль ${value}`;
                    } else {
                        translatedObj[translatedKey] = translateKeys(value);
                    }
                }
                return translatedObj;
            } else {
                return enumTranslations[input] || input;
            }
        }

        const translatedObj = translateKeys(obj);
        return JSON.stringify(translatedObj, null, 2);
    } catch (e) {
        return jsonString;
    }
}

// Экспортируем функции для использования в других скриптах
window.translateActionType = translateActionType;
window.translateDescription = translateDescription;
window.translateColumnHeader = translateColumnHeader;
window.translateFieldNames = translateFieldNames;