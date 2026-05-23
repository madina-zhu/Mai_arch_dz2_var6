# 📦 Delivery Service API — Лабораторная работа 2

> REST API сервис для управления доставкой (аналог CDEK) с in-memory хранилищем

---

## О проекте

Delivery Service API — учебный backend-сервис, разработанный в рамках курса «Программная инженерия».

**Вариант задания:** №6 — Сервис доставки (Пользователь, Посылка, Доставка)

**Функционал:**
- 👤 Пользователи — регистрация, логин, поиск
- 📦 Посылки — создание, получение по отправителю
- 🚚 Доставки — создание, получение по отправителю/получателю
- 🔐 JWT-аутентификация

---

## Технологии

| Технология | Назначение |
|:---|:---|
| C++20 | Язык программирования |
| Yandex Userver | Фреймворк для высоконагруженных сервисов |
| JWT (jwt-cpp, HS256) | Аутентификация |
| In-memory (std::map) | Хранилище данных |
| Docker, docker-compose | Контейнеризация |
| OpenAPI 3.0 + Swagger UI | Документация API |
| pytest + requests | Тестирование |

---

## Быстрый старт

### Требования
- Docker + Docker Compose
- 2+ GB RAM

### Запуск

```bash
# Перейти в директорию ЛР-2
cd delivery-api-lr-2

# Сборка и запуск
docker-compose up --build

# Открыть Swagger UI
http://localhost:8080/api/v1/docs
```

### Остановка

```bash
docker-compose down
```

---

## Архитектура данных (In-memory)

Данные хранятся в оперативной памяти с использованием `std::map`.

### Структуры данных

```cpp
// Пользователь
struct UserData {
    int64_t id;
    UserCreateRequest request;
    std::string password_hash;
    std::string created_at;
};

// Посылка
struct ParcelData {
    int64_t id;
    ParcelCreateRequest request;
    std::string status;
    std::string created_at;
};

// Доставка
struct DeliveryData {
    int64_t id;
    DeliveryCreateRequest request;
    std::string tracking_number;
    std::string status;
    std::string created_at;
    std::optional<std::string> delivered_at;
};
```

---

## API Endpoints

Базовый URL: `http://localhost:8080/api/v1`

### Аутентификация и Пользователи

| Метод | Путь | Описание | Аутентификация |
|:---|:---|:---|:---:|
| `POST` | `/users` | Регистрация пользователя | ❌ |
| `POST` | `/login` | Получение JWT токена | ❌ |
| `GET` | `/users?login=...` | Поиск пользователя по логину | ❌ |
| `GET` | `/users?name_mask=...` | Поиск по маске имени/фамилии | ❌ |

### Посылки

| Метод | Путь | Описание | Аутентификация |
|:---|:---|:---|:---:|
| `POST` | `/parcels` | Создание посылки | ✅ |
| `GET` | `/parcels?user_id=...` | Получение посылок пользователя | ❌ |

### Доставки

| Метод | Путь | Описание | Аутентификация |
|:---|:---|:---|:---:|
| `POST` | `/deliveries` | Создание доставки | ✅ |
| `GET` | `/deliveries?tracking_number=...` | Поиск по трек-номеру | ❌ |
| `GET` | `/deliveries?sender_id=...` | Доставки отправителя | ❌ |
| `GET` | `/deliveries?receiver_id=...` | Доставки получателя | ❌ |

### Документация

| Метод | Путь | Описание |
|:---|:---|:---|
| `GET` | `/docs` | Swagger UI |
| `GET` | `/docs/openapi.yaml` | OpenAPI спецификация |

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

**Ответ (201 Created):**
```json
{
    "id": 1,
    "login": "john_doe",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com",
    "created_at": "2026-05-21T10:00:00Z"
}
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

**Ответ (200 OK):**
```json
{
    "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "token_type": "Bearer",
    "user_id": 1,
    "login": "john_doe"
}
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
    "declared_value": 5000.00,
    "description": "Книги и документы"
  }'
```

**Ответ (201 Created):**
```json
{
    "id": 1,
    "sender_id": 1,
    "weight": 2.5,
    "dimensions": "30x20x15",
    "declared_value": 5000.0,
    "description": "Книги и документы",
    "status": "created",
    "created_at": "2026-05-21T10:05:00Z"
}
```

### 4. Создание доставки

```bash
curl -X POST http://localhost:8080/api/v1/deliveries \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "parcel_id": 1,
    "receiver_id": 2,
    "from_address": "Москва, Тверская 15",
    "to_address": "Санкт-Петербург, Невский 25"
  }'
```

**Ответ (201 Created):**
```json
{
    "id": 1,
    "parcel_id": 1,
    "sender_id": 1,
    "receiver_id": 2,
    "tracking_number": "DEL-1-ABC123",
    "status": "pending",
    "from_address": "Москва, Тверская 15",
    "to_address": "Санкт-Петербург, Невский 25",
    "created_at": "2026-05-21T10:10:00Z",
    "delivered_at": null
}
```

### 5. Поиск доставок отправителя

```bash
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

**Ответ (200 OK):**
```json
[
    {
        "id": 1,
        "parcel_id": 1,
        "sender_id": 1,
        "receiver_id": 2,
        "tracking_number": "DEL-1-ABC123",
        "status": "pending",
        "from_address": "Москва, Тверская 15",
        "to_address": "Санкт-Петербург, Невский 25",
        "created_at": "2026-05-21T10:10:00Z"
    }
]
```

### 6. Поиск по трек-номеру

```bash
curl "http://localhost:8080/api/v1/deliveries?tracking_number=DEL-1-ABC123"
```

---

## 📊 Коды ответов

| Код | Описание |
|:---:|:---|
| 200 | Успешный запрос |
| 201 | Ресурс создан |
| 400 | Ошибка валидации |
| 401 | Не авторизован |
| 403 | Доступ запрещен |
| 404 | Ресурс не найден |
| 409 | Конфликт (логин уже существует) |

---

## 📁 Структура проекта

```
delivery-api-lr-2/
├── src/
│   ├── components/
│   │   ├── auth_component.cpp/hpp      # JWT аутентификация
│   │   └── storage_component.cpp/hpp   # In-memory хранилище
│   ├── handlers/
│   │   ├── create_parcel_handler.cpp/hpp
│   │   ├── create_user_handler.cpp/hpp
│   │   ├── create_delivery_handler.cpp/hpp
│   │   ├── get_parcels_handler.cpp/hpp
│   │   ├── get_deliveries_handler.cpp/hpp
│   │   ├── get_users_handler.cpp/hpp
│   │   ├── login_handler.cpp/hpp
│   │   ├── openapi_handler.cpp/hpp
│   │   └── swagger_ui_handler.cpp/hpp
│   ├── jwt_auth/                       # JWT проверка
│   ├── models/                         # DTO структуры
│   └── main.cpp
├── configs/
│   ├── static_config.yaml              # Конфигурация Userver
│   └── openapi.yaml                    # OpenAPI спецификация
├── tests/
│   ├── test_api.py                     # pytest тесты
│   └── requirements.txt
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── README.md
```

---

## Тестирование

### Запуск тестов

```bash
# Установить зависимости
pip install -r tests/requirements.txt

# Запустить тесты
pytest tests/test_api.py -v

# С отчетом о покрытии
pytest tests/test_api.py -v --cov=. --cov-report=html
```

### Структура тестов

| Класс | Описание |
|:---|:---|
| `TestAuthEndpoints` | Проверка логина (успех/ошибка) |
| `TestUserEndpoints` | Регистрация, поиск пользователей |
| `TestParcelEndpoints` | Создание, получение посылок |
| `TestDeliveryEndpoints` | Создание, поиск доставок |
| `TestDocsEndpoints` | Swagger UI, OpenAPI |
| `TestEdgeCases` | Интеграционные сценарии |

---

## Документация

- **Swagger UI:** `http://localhost:8080/api/v1/docs`
- **OpenAPI спецификация:** `http://localhost:8080/api/v1/docs/openapi.yaml`

