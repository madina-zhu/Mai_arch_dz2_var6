-- ============================================================================
-- SQL запросы для Delivery API (вариант 6: Доставка)
-- ============================================================================

-- 1. Создание нового пользователя
INSERT INTO users (login, password_hash, first_name, last_name, email)
VALUES ('new_user', 'hash_password', 'Peter', 'Parker', 'peter@example.com')
RETURNING id, login, first_name, last_name, email, created_at;

-- 2. Поиск пользователя по логину (точное совпадение)
SELECT id, login, first_name, last_name, email, created_at
FROM users
WHERE login = 'john_doe';

-- 3. Поиск пользователя по маске имени и фамилии
SELECT id, login, first_name, last_name, email, created_at
FROM users
WHERE first_name LIKE 'Ivan%' OR last_name LIKE 'Ivan%';

-- 4. Создание посылки
INSERT INTO parcels (sender_id, weight, dimensions, declared_value, description, status)
VALUES (1, 2.5, '30x20x15', 5000.00, 'Books', 'created')
RETURNING id, sender_id, weight, dimensions, declared_value, description, status, created_at;

-- 5. Получение посылок пользователя
SELECT id, sender_id, weight, dimensions, declared_value, description, status, created_at
FROM parcels
WHERE sender_id = 1
ORDER BY created_at DESC;

-- 6. Создание доставки от пользователя к пользователю
INSERT INTO deliveries (parcel_id, sender_id, receiver_id, tracking_number, status, from_address, to_address)
VALUES (1, 1, 2, 'DEL-001-ABC123', 'pending', 'Moscow, Tverskaya 15', 'SPb, Nevsky 25')
RETURNING id, parcel_id, sender_id, receiver_id, tracking_number, status, from_address, to_address, created_at;

-- 7. Получение информации о доставке по получателю
SELECT d.id, d.parcel_id, d.sender_id, d.receiver_id, d.tracking_number, d.status, 
       d.from_address, d.to_address, d.created_at, d.delivered_at,
       p.weight, p.dimensions, p.declared_value
FROM deliveries d
JOIN parcels p ON d.parcel_id = p.id
WHERE d.receiver_id = 2
ORDER BY d.created_at DESC;

-- 8. Получение информации о доставке по отправителю
SELECT d.id, d.parcel_id, d.sender_id, d.receiver_id, d.tracking_number, d.status, 
       d.from_address, d.to_address, d.created_at, d.delivered_at,
       p.weight, p.dimensions, p.declared_value
FROM deliveries d
JOIN parcels p ON d.parcel_id = p.id
WHERE d.sender_id = 1
ORDER BY d.created_at DESC;

-- 9. Получение доставки по трек-номеру
SELECT d.id, d.parcel_id, d.sender_id, d.receiver_id, d.tracking_number, d.status, 
       d.from_address, d.to_address, d.created_at, d.delivered_at,
       p.weight, p.dimensions, p.declared_value, p.description
FROM deliveries d
JOIN parcels p ON d.parcel_id = p.id
WHERE d.tracking_number = 'DEL-001-ABC123';

-- 10. Обновление статуса доставки
UPDATE deliveries
SET status = 'delivered', delivered_at = NOW()
WHERE tracking_number = 'DEL-001-ABC123'
RETURNING id, status, delivered_at;

-- 11. Статистика: количество доставок по статусу
SELECT status, COUNT(*) as count
FROM deliveries
GROUP BY status
ORDER BY count DESC;

-- 12. Статистика: среднее время доставки (для доставленных)
SELECT AVG(EXTRACT(EPOCH FROM (delivered_at - created_at)) / 3600) as avg_delivery_hours
FROM deliveries
WHERE status = 'delivered';

-- 13. Поиск доставок с фильтром по статусу и дате (использует партиционирование)
SELECT id, tracking_number, status, from_address, to_address, created_at
FROM deliveries
WHERE status = 'pending' 
  AND created_at >= NOW() - INTERVAL '7 days'
ORDER BY created_at DESC
LIMIT 50;