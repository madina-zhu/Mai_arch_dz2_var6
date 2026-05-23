# 🗄️ Delivery Service API — Лабораторная работа 3

> Подключение PostgreSQL, оптимизация запросов и партиционирование

---

## О проекте

В данной лабораторной работе реализовано подключение реляционной базы данных **PostgreSQL 16** для хранения пользователей. Произведена оптимизация запросов с помощью индексов, анализ планов выполнения через `EXPLAIN ANALYZE`, а также применено партиционирование таблицы для повышения производительности.

**Вариант задания:** №6 — Сервис доставки (Пользователь, Посылка, Доставка)

**Функционал:**
- 👤 Пользователи — регистрация, логин, поиск (PostgreSQL)
- 📦 Посылки — создание, получение по отправителю (PostgreSQL)
- 🚚 Доставки — создание, получение по отправителю/получателю (PostgreSQL с партиционированием)
- 🔐 JWT-аутентификация

---

## Технологии

| Технология | Версия | Назначение |
|:---|:---:|:---|
| C++20 | - | Язык программирования |
| Yandex Userver | latest | Фреймворк |
| PostgreSQL | 16 | Реляционная база данных |
| JWT (jwt-cpp) | 0.7.2 | Аутентификация |
| Docker, docker-compose | - | Контейнеризация |
| OpenAPI 3.0 + Swagger UI | - | Документация API |
| pytest + requests | - | Тестирование |

---

## Быстрый старт

### Запуск

```bash
# Перейти в директорию ЛР-3
cd delivery-api-lr-3

# Сборка и запуск
docker-compose up --build

# Открыть Swagger UI
http://localhost:8080/api/v1/docs
```

### Заполнение тестовыми данными

```bash
# Для Linux/macOS
cat data.sql | docker exec -i postgres_delivery psql -U delivery_user -d delivery_db

# Для Windows PowerShell
Get-Content data.sql | docker exec -i postgres_delivery psql -U delivery_user -d delivery_db
```

---

## Схема базы данных

### Таблица `users`

| Колонка | Тип | Ограничения | Описание |
|:---|:---|:---|:---|
| `id` | BIGINT | PRIMARY KEY, AUTO_INCREMENT | Уникальный ID |
| `login` | VARCHAR(50) | NOT NULL, UNIQUE | Логин (3-50 символов) |
| `password_hash` | VARCHAR(255) | NOT NULL | Хэш пароля |
| `first_name` | VARCHAR(100) | NOT NULL | Имя |
| `last_name` | VARCHAR(100) | NOT NULL | Фамилия |
| `email` | VARCHAR(255) | NOT NULL, UNIQUE | Email |
| `created_at` | TIMESTAMPTZ | DEFAULT NOW() | Дата регистрации |

### Таблица `parcels` (посылки)

| Колонка | Тип | Ограничения | Описание |
|:---|:---|:---|:---|
| `id` | BIGINT | PRIMARY KEY | Уникальный ID |
| `sender_id` | BIGINT | FK → users(id) | Отправитель |
| `weight` | DOUBLE PRECISION | > 0 | Вес в кг |
| `dimensions` | VARCHAR(50) | NOT NULL | Размеры (30x20x15) |
| `declared_value` | DECIMAL(12,2) | ≥ 0 | Объявленная стоимость |
| `description` | TEXT | - | Описание |
| `status` | VARCHAR(20) | created/assigned/in_transit/delivered | Статус |
| `created_at` | TIMESTAMPTZ | DEFAULT NOW() | Дата создания |

### Таблица `deliveries` (доставки с партиционированием)

| Колонка | Тип | Ограничения | Описание |
|:---|:---|:---|:---|
| `id` | BIGINT | PRIMARY KEY (с created_at) | Уникальный ID |
| `parcel_id` | BIGINT | FK → parcels(id) | ID посылки |
| `sender_id` | BIGINT | FK → users(id) | Отправитель |
| `receiver_id` | BIGINT | FK → users(id) | Получатель |
| `tracking_number` | VARCHAR(50) | NOT NULL, UNIQUE | Трек-номер |
| `status` | VARCHAR(20) | pending/in_transit/delivered/cancelled | Статус |
| `from_address` | TEXT | NOT NULL | Адрес отправления |
| `to_address` | TEXT | NOT NULL | Адрес назначения |
| `created_at` | TIMESTAMPTZ | NOT NULL (ключ партиционирования) | Дата создания |
| `delivered_at` | TIMESTAMPTZ | NULL | Дата доставки |

**Партиционирование:** По диапазону `created_at` (помесячно):

```sql
deliveries_2025_10, deliveries_2025_11, deliveries_2025_12,
deliveries_2026_01, deliveries_2026_02, ..., deliveries_2026_06,
deliveries_default  -- резервная партиция
```

---

## Оптимизация запросов

### Созданные индексы

| Индекс | Тип | Таблица | Колонки | Назначение |
|:---|:---|:---|:---|:---|
| `idx_users_login` | UNIQUE B-Tree | users | login | Поиск по логину |
| `idx_users_first_trgm` | GIN | users | first_name | Поиск по маске имени |
| `idx_users_last_trgm` | GIN | users | last_name | Поиск по маске фамилии |
| `idx_parcels_sender_id` | B-Tree | parcels | sender_id | Фильтр по отправителю |
| `idx_deliveries_tracking` | UNIQUE B-Tree | deliveries | tracking_number | Поиск по трек-номеру |
| `idx_deliveries_sender_id` | B-Tree | deliveries | sender_id | Фильтр по отправителю |
| `idx_deliveries_receiver_id` | B-Tree | deliveries | receiver_id | Фильтр по получателю |
| `idx_deliveries_status_created` | B-Tree | deliveries | status, created_at DESC | Фильтр по статусу + сортировка |
| `idx_deliveries_sender_covering` | Covering | deliveries | sender_id, created_at DESC | Покрывающий индекс (INCLUDE) |

### Результаты оптимизации

| Запрос | До оптимизации | После оптимизации | Ускорение |
|:---|:---:|:---:|:---:|
| Поиск пользователя по маске | ~24 мс | ~3.8 мс | **~6x** |
| Поиск доставок по отправителю | ~245 мс | ~0.2 мс | **~1200x** |
| Поиск по трек-номеру | ~123 мс | ~0.07 мс | **~1700x** |
| Фильтр по статусу и дате | ~157 мс | ~0.1 мс | **~1500x** |
| Посылки пользователя | ~46 мс | ~0.6 мс | **~80x** |

---

## API Endpoints

Базовый URL: `http://localhost:8080/api/v1`

| Метод | Путь | Описание | Аутентификация |
|:---|:---|:---|:---:|
| `POST` | `/users` | Регистрация | ❌ |
| `POST` | `/login` | Получение JWT | ❌ |
| `GET` | `/users` | Поиск пользователей (login/name_mask) | ❌ |
| `POST` | `/parcels` | Создание посылки | ✅ |
| `GET` | `/parcels` | Посылки пользователя | ❌ |
| `POST` | `/deliveries` | Создание доставки | ✅ |
| `GET` | `/deliveries` | Поиск доставок (tracking/sender/receiver) | ❌ |
| `GET` | `/docs` | Swagger UI | ❌ |
| `GET` | `/docs/openapi.yaml` | OpenAPI спецификация | ❌ |

---

## Примеры запросов

### 1. Регистрация пользователя

```bash
curl -X POST http://localhost:8080/api/v1/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secure123",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com"
  }'
```

### 2. Получение JWT токена

```bash
curl -X POST http://localhost:8080/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secure123"
  }'
```

### 3. Создание посылки (с токеном)

```bash
TOKEN="ваш_jwt_токен"

curl -X POST http://localhost:8080/api/v1/parcels \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "sender_id": 1,
    "weight": 2.5,
    "dimensions": "30x20x15",
    "declared_value": 5000,
    "description": "Книги"
  }'
```

### 4. Создание доставки

```bash
curl -X POST http://localhost:8080/api/v1/deliveries \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "parcel_id": 1,
    "sender_id": 1,
    "receiver_id": 2,
    "from_address": "Москва, Тверская 15",
    "to_address": "СПб, Невский 25"
  }'
```

### 5. Поиск доставки по трек-номеру

```bash
curl "http://localhost:8080/api/v1/deliveries?tracking_number=DEL-1-ABC123"
```

---

## 📁 Структура проекта

```
delivery-api-lr-3/
├── src/
│   ├── components/
│   │   ├── auth_component.cpp/hpp          # JWT аутентификация
│   │   └── postgres_storage_component.cpp/hpp  # PostgreSQL хранилище
│   ├── handlers/                           # Обработчики запросов
│   ├── jwt_auth/                           # JWT проверка
│   ├── models/                             # DTO структуры
│   └── main.cpp
├── configs/
│   ├── static_config.yaml
│   └── openapi.yaml
├── tests/
│   ├── test_api.py
│   └── requirements.txt
├── schema.sql          # Создание таблиц и индексов
├── data.sql            # Тестовые данные (10 пользователей)
├── queries.sql         # SQL запросы для всех операций
├── optimization.md     # Отчет по оптимизации с EXPLAIN
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── README.md
```

---

## Тестирование

```bash
pip install -r tests/requirements.txt
pytest tests/test_api.py -v
```

---

## Документация

- **Swagger UI:** `http://localhost:8080/api/v1/docs`
- **OpenAPI спецификация:** `http://localhost:8080/api/v1/docs/openapi.yaml`
- **Отчет по оптимизации:** [optimization.md](optimization.md)

---
