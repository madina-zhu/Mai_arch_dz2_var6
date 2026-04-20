```markdown
# 📦 Delivery Service API

> REST API сервис для управления доставкой посылок (аналог СДЭК, DHL)

---

## 📦 О проекте

Delivery Service API — учебный backend-сервис, разработанный в рамках курса «Программная инженерия» студентом группы М8О-105СВ-25

**Функционал:**
- 👤 Пользователи — регистрация, логин, поиск
- 📦 Посылки — создание, получение посылок пользователя
- 🚚 Доставки — создание доставки от пользователя к пользователю
- 🔐 JWT-аутентификация

**Вариант задания:** №6 — Сервис доставки (СДЭК/DHL)

---

## ⚙️ Технологии

При выполнении задания лабораторной работы были использованы следующие технологии:

- C++20
- Yandex Userver
- JWT (jwt-cpp, HS256)
- PostgreSQL 16
- MongoDB 7
- Docker, docker-compose
- OpenAPI 3.0 + Swagger UI
- pytest + requests

---

## 🚀 Быстрый старт

**Требования:**
- Docker + Docker Compose

**Первый старт:**
```bash
docker-compose up --build
```

**Последующие старты:**
```bash
docker-compose up
```

**Открыть Swagger UI:**
```
http://localhost:8080/api/v1/docs
```

---

## 🗄️ Архитектура хранения данных

Проект использует **гибридный подход** к хранению данных, сочетая реляционную СУБД PostgreSQL 16 для структурированных данных пользователей и документоориентированную базу данных MongoDB 7 для гибких сущностей посылок и доставок.

### PostgreSQL: Таблица users

Таблица предназначена для хранения данных пользователей.

**Таблица `users`**: Хранит данные пользователей.

| Поле | Тип | Описание |
|------|-----|----------|
| `id` | BIGINT | Уникальный идентификатор (автоинкремент) |
| `login` | VARCHAR(50) | Уникальный логин (3-50 символов) |
| `password_hash` | VARCHAR(255) | Хэш пароля |
| `first_name` | VARCHAR(100) | Имя пользователя |
| `last_name` | VARCHAR(100) | Фамилия пользователя |
| `email` | VARCHAR(255) | Уникальный email |
| `created_at` | TIMESTAMP | Дата регистрации |

### MongoDB: Коллекции

Использование MongoDB обусловлено необходимостью хранения динамических структур данных (различные типы посылок) и эффективной работы с массивами вложенных данных.

#### Коллекция `parcels`

Хранит информацию о посылках. Благодаря схеме NoSQL, структура документа может варьироваться в зависимости от типа посылки (обычная, хрупкая, крупногабаритная, экспресс).

| Поле | Тип | Обязательное | Описание |
|------|-----|--------------|----------|
| `_id` | String | Да | Уникальный идентификатор документа |
| `sender_id` | Int32/Int64 | Да | ID отправителя (ссылка на PostgreSQL) |
| `recipient_id` | Int32/Int64 | Да | ID получателя (ссылка на PostgreSQL) |
| `type` | String | Да | Тип посылки: "standard", "fragile", "oversized", "express" |
| `title` | String | Да | Название/описание посылки |
| `from_city` | String | Да | Город отправления |
| `to_city` | String | Да | Город назначения |
| `address` | Object | Да | Адрес отправителя |
| `weight` | Number | Да | Вес в килограммах (>0) |
| `price` | Number | Да | Стоимость доставки |
| `status` | String | Да | Статус: "pending", "in_transit", "delivered", "cancelled" |
| `features` | Array[String] | Нет | Массив строк с особенностями |
| `created_at` | Date | Нет | Дата создания |

#### Коллекция `deliveries`

Хранит информацию о доставках (логистические задания).

| Поле | Тип | Обязательное | Описание |
|------|-----|--------------|----------|
| `_id` | String | Да | Уникальный идентификатор |
| `parcel_id` | ObjectId | Да | Ссылка на документ в коллекции parcels |
| `courier_id` | Int32/Int64 | Да | ID курьера |
| `pickup_time` | Date | Да | Время забора посылки |
| `estimated_delivery` | Date | Да | Ожидаемая дата доставки |
| `actual_delivery` | Date | Нет | Фактическая дата доставки |
| `tracking_history` | Array[Object] | Да | Массив событий трекинга |
| `status` | String | Нет | Статус: "pending", "assigned", "in_transit", "delivered", "cancelled" |
| `created_at` | Date | Нет | Дата создания |

---

## 💾 Инициализация баз данных

Для заполнения PostgreSQL и MongoDB тестовыми данными выполните команды, приведенные ниже.

**Для Linux / macOS:**
```bash
cat data.sql | docker exec -i postgres_delivery psql -U api_user -d delivery_db
cat data.js | docker exec -i mongodb_delivery mongosh -u admin -p password
```

**Для PowerShell (Windows):**
```powershell
Get-Content data.sql | docker exec -i postgres_delivery psql -U api_user -d delivery_db
Get-Content data.js | docker exec -i mongodb_delivery mongosh -u admin -p password
```

---

## 📡 API Endpoints

**Базовый URL:**
```
http://localhost:8080/api/v1
```

### Аутентификация и Пользователи

| Метод | Путь | Краткое описание | Аутентификация |
| :---- | :--- | :--------------- | :------------: |
| `POST` | `/users` | Создание пользователя (регистрация) | ❌ |
| `POST` | `/login` | Получение JWT токена | ❌ |
| `GET` | `/users?login=...` | Поиск по логину | ❌ |
| `GET` | `/users?name_mask=...` | Поиск по маске имени/фамилии | ❌ |

### Посылки

| Метод | Путь | Краткое описание | Аутентификация |
| :---- | :--- | :--------------- | :------------: |
| `POST` | `/parcels` | Создание посылки | ✅ |
| `GET` | `/parcels?sender_id=...` | Получение посылок пользователя (отправителя) | ❌ |
| `GET` | `/parcels?from_city=...` | Поиск по городу отправления | ❌ |
| `GET` | `/parcels?min_weight=...&max_weight=...` | Поиск по диапазону веса | ❌ |

### Доставки

| Метод | Путь | Краткое описание | Аутентификация |
| :---- | :--- | :--------------- | :------------: |
| `POST` | `/deliveries` | Создание доставки от пользователя к пользователю | ✅ |
| `GET` | `/deliveries?recipient_id=...` | Получение доставок по получателю | ❌ |
| `GET` | `/deliveries?sender_id=...` | Получение доставок по отправителю | ❌ |

### Документация

| Метод | Путь | Краткое описание |
| :---- | :--- | :--------------- |
| `GET` | `/docs` | Swagger UI |
| `GET` | `/docs/openapi.yaml` | OpenAPI спецификация |

> 💡 **Подробное описание** всех эндпоинтов, схем запросов и ответов смотрите в Swagger UI.

---

## 🔐 Аутентификация и Регистрация

Получение токена состоит из двух этапов.

### 1. Регистрация

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

### 2. Аутентификация

```bash
curl -X POST http://localhost:8080/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secure123"
  }'
```

### 3. Результат

```json
{
  "access_token": "JWT_TOKEN",
  "token_type": "Bearer",
  "user_id": 1,
  "login": "john_doe"
}
```

---

## 🔑 Использование токена

Пример создания посылки с использованием токена:

```bash
curl -X POST http://localhost:8080/api/v1/parcels \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <TOKEN>" \
  -d '{
    "sender_id": 1,
    "recipient_id": 2,
    "type": "standard",
    "title": "Документы",
    "from_city": "Moscow",
    "to_city": "Saint Petersburg",
    "address": {
      "street": "Tverskaya",
      "house": "10",
      "flat": "5"
    },
    "weight": 0.5,
    "price": 500,
    "details": {
      "is_fragile": false
    },
    "status": "pending"
  }'
```

---

## 📝 Примеры запросов

### Регистрация

```bash
curl -X POST http://localhost:8080/api/v1/users \
  -d '{
    "login": "alice_wonder",
    "password": "wonderland123",
    "first_name": "Alice",
    "last_name": "Wonder",
    "email": "alice@example.com"
  }'
```

### Поиск посылок по городу отправления

```bash
curl "http://localhost:8080/api/v1/parcels?from_city=Moscow"
```

### Создание доставки

```bash
curl -X POST http://localhost:8080/api/v1/deliveries \
  -H "Authorization: Bearer <TOKEN>" \
  -d '{
    "parcel_id": "19936beec3de4b099bc3017f27cd8013",
    "courier_id": 101,
    "pickup_time": "2026-04-15T10:00:00Z",
    "estimated_delivery": "2026-04-16T18:00:00Z"
  }'
```

### Получение доставок по получателю

```bash
curl "http://localhost:8080/api/v1/deliveries?recipient_id=2"
```

### Получение доставок по отправителю

```bash
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

### Примеры ошибок

**404 — Пользователь не найден:**
```json
{
  "code": "NOT_FOUND",
  "message": "User not found"
}
```

**401 — Неверные учетные данные:**
```json
{
  "code": "UNAUTHORIZED",
  "message": "Invalid credentials"
}
```

---

## 🧪 Тестирование

### Запуск тестов API

```bash
pip install -r tests/requirements.txt  
pytest tests/test_api.py -v  
```

### Запуск aggregation pipeline (MongoDB)

```bash
docker exec -i mongodb_delivery mongosh -u admin -p password < aggregation.js
```

### Тестирование взаимодействия с MongoDB

```bash
docker exec -i mongodb_delivery mongosh -u admin -p password
load("./queries.js");
```

Примеры запросов в MongoDB shell:

```javascript
findParcelsBySender(1);
findDeliveriesByRecipient(2);
```

---

## 📚 Дополнительная документация

| Документ | Описание |
|----------|----------|
| [Swagger UI](http://localhost:8080/api/v1/docs) | Интерактивная документация API |
| [OpenAPI спецификация](http://localhost:8080/api/v1/docs/openapi.yaml) | Спецификация API в формате YAML |
| [optimization.md](optimization.md) | Оптимизация PostgreSQL БД |
| [schema_design.md](schema_design.md) | Описание проектирования MongoDB |

---

## 📁 Структура проекта

```
delivery-service-api/
├── configs/
│   ├── openapi.yaml          # OpenAPI спецификация
│   └── static_config.yaml    # Конфигурация userver
├── src/
│   ├── components/           # Компоненты (PostgreSQL + MongoDB)
│   ├── handlers/             # HTTP обработчики
│   ├── jwt_auth/             # JWT аутентификация
│   └── models/               # DTO модели
├── tests/
│   ├── test_api.py           # Интеграционные тесты
│   └── requirements.txt      # Зависимости для тестов
├── schema.sql                # Схема PostgreSQL (только users)
├── data.sql                  # Тестовые данные PostgreSQL
├── data.js                   # Тестовые данные MongoDB
├── queries.js                # MongoDB запросы
├── validation.js             # Валидация схем MongoDB
├── aggregation.js            # Агрегации MongoDB
├── schema_design.md          # Обоснование документной модели
├── optimization.md           # Отчет по оптимизации БД
├── Dockerfile                # Продакшн сборка
├── Dockerfile.dev            # Dev контейнер
├── docker-compose.yml        # Продакшн запуск
├── docker-compose.dev.yml    # Dev запуск
├── CMakeLists.txt            # Сборка проекта
└── README.md                 # Документация
