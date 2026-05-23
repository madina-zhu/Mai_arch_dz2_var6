-- ============================================================================
-- Тестовые данные для Delivery API
-- ============================================================================

TRUNCATE TABLE deliveries RESTART IDENTITY CASCADE;
TRUNCATE TABLE parcels RESTART IDENTITY CASCADE;
TRUNCATE TABLE users RESTART IDENTITY CASCADE;

-- ============================================================================
-- Пользователи (10 записей)
-- ============================================================================
INSERT INTO users (login, password_hash, first_name, last_name, email, created_at) VALUES
('ivan', 'hash_ivan123', 'Иван', 'Петров', 'ivan@example.com', NOW() - INTERVAL '10 days'),
('maria', 'hash_maria123', 'Мария', 'Смирнова', 'maria@example.com', NOW() - INTERVAL '9 days'),
('alexey', 'hash_alexey123', 'Алексей', 'Козлов', 'alexey@example.com', NOW() - INTERVAL '8 days'),
('elena', 'hash_elena123', 'Елена', 'Соколова', 'elena@example.com', NOW() - INTERVAL '7 days'),
('dmitry', 'hash_dmitry123', 'Дмитрий', 'Морозов', 'dmitry@example.com', NOW() - INTERVAL '6 days'),
('anna', 'hash_anna123', 'Анна', 'Волкова', 'anna@example.com', NOW() - INTERVAL '5 days'),
('sergey', 'hash_sergey123', 'Сергей', 'Новиков', 'sergey@example.com', NOW() - INTERVAL '4 days'),
('olga', 'hash_olga123', 'Ольга', 'Федорова', 'olga@example.com', NOW() - INTERVAL '3 days'),
('maxim', 'hash_maxim123', 'Максим', 'Егоров', 'maxim@example.com', NOW() - INTERVAL '2 days'),
('natalia', 'hash_natalia123', 'Наталья', 'Павлова', 'natalia@example.com', NOW() - INTERVAL '1 day');

-- ============================================================================
-- Посылки (10 записей)
-- ============================================================================
INSERT INTO parcels (sender_id, weight, dimensions, declared_value, description, status, created_at) VALUES
(1, 2.5, '30x20x15', 5000.00, 'Книги и документы', 'created', NOW() - INTERVAL '8 days'),
(1, 5.0, '40x30x20', 15000.00, 'Ноутбук и аксессуары', 'assigned', NOW() - INTERVAL '7 days'),
(2, 1.2, '25x15x10', 3000.00, 'Одежда', 'in_transit', NOW() - INTERVAL '6 days'),
(2, 3.8, '35x25x20', 8000.00, 'Посуда', 'delivered', NOW() - INTERVAL '5 days'),
(3, 10.0, '50x40x30', 25000.00, 'Бытовая техника', 'created', NOW() - INTERVAL '4 days'),
(3, 0.8, '20x15x10', 1000.00, 'Косметика', 'assigned', NOW() - INTERVAL '3 days'),
(4, 7.5, '45x35x25', 12000.00, 'Спортивный инвентарь', 'in_transit', NOW() - INTERVAL '2 days'),
(5, 2.0, '30x20x15', 4500.00, 'Игрушки', 'delivered', NOW() - INTERVAL '1 day'),
(6, 4.2, '40x30x20', 9500.00, 'Книги', 'created', NOW()),
(7, 6.0, '50x40x30', 18000.00, 'Электроника', 'assigned', NOW());

-- ============================================================================
-- Доставки (10 записей)
-- ============================================================================
INSERT INTO deliveries (parcel_id, sender_id, receiver_id, tracking_number, status, from_address, to_address, created_at, delivered_at) VALUES
(1, 1, 2, 'DEL-001-ABC123', 'delivered', 'Москва, Тверская 15', 'Санкт-Петербург, Невский 25', NOW() - INTERVAL '8 days', NOW() - INTERVAL '6 days'),
(2, 1, 3, 'DEL-002-DEF456', 'in_transit', 'Москва, Ленинский пр-т 10', 'Казань, Баумана 30', NOW() - INTERVAL '7 days', NULL),
(3, 2, 4, 'DEL-003-GHI789', 'delivered', 'Новосибирск, Красный пр-т 50', 'Екатеринбург, Ленина 12', NOW() - INTERVAL '6 days', NOW() - INTERVAL '4 days'),
(4, 2, 5, 'DEL-004-JKL012', 'pending', 'Екатеринбург, Мира 8', 'Москва, Арбат 20', NOW() - INTERVAL '5 days', NULL),
(5, 3, 6, 'DEL-005-MNO345', 'in_transit', 'Казань, Кремлевская 5', 'Новосибирск, Советская 40', NOW() - INTERVAL '4 days', NULL),
(6, 3, 7, 'DEL-006-PQR678', 'pending', 'Санкт-Петербург, Невский 100', 'Москва, Тверская 15', NOW() - INTERVAL '3 days', NULL),
(7, 4, 8, 'DEL-007-STU901', 'delivered', 'Москва, Кутузовский пр-т 30', 'Сочи, Курортный 15', NOW() - INTERVAL '2 days', NOW()),
(8, 5, 9, 'DEL-008-VWX234', 'cancelled', 'Екатеринбург, Ленина 50', 'Москва, Мира 10', NOW() - INTERVAL '1 day', NULL),
(9, 6, 10, 'DEL-009-YZA567', 'pending', 'Новосибирск, Дмитрия Донского 12', 'Казань, Япеева 8', NOW(), NULL),
(10, 7, 1, 'DEL-010-BCD890', 'in_transit', 'Сочи, Курортный 5', 'Москва, Тверская 15', NOW(), NULL);

ANALYZE;