-- ============================================================================
-- Data Generation: Delivery Database
-- Description: Заполнение базы данных тестовыми пользователями
-- ============================================================================

TRUNCATE TABLE users RESTART IDENTITY CASCADE;
ANALYZE;

INSERT INTO users (login, password_hash, first_name, last_name, email, created_at) VALUES
('ivan', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Ivan', 'Ivanov', 'ivan@example.com', NOW() - INTERVAL '10 days'),
('maria', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Maria', 'Smirnova', 'maria@example.com', NOW() - INTERVAL '9 days'),
('alexey', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Alexey', 'Petrov', 'alexey@example.com', NOW() - INTERVAL '8 days'),
('elena', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Elena', 'Sokolova', 'elena@example.com', NOW() - INTERVAL '7 days'),
('dmitry', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Dmitry', 'Kuznetsov', 'dmitry@example.com', NOW() - INTERVAL '6 days'),
('anna', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Anna', 'Vasilieva', 'anna@example.com', NOW() - INTERVAL '5 days'),
('sergey', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Sergey', 'Popov', 'sergey@example.com', NOW() - INTERVAL '4 days'),
('olga', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Olga', 'Novikova', 'olga@example.com', NOW() - INTERVAL '3 days'),
('maxim', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Maxim', 'Fedorov', 'maxim@example.com', NOW() - INTERVAL '2 days'),
('natalia', '$2b$12$LJ3m4ys3Q5w6z7x8c9v0b.N1o2p3q4r5s6t7u8v9w0x1y2z3a4b5c', 'Natalia', 'Morozova', 'natalia@example.com', NOW() - INTERVAL '1 day');

ANALYZE users;