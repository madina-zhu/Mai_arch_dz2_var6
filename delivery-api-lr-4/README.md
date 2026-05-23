# 📦 Delivery Service API — Лабораторная работа 4

> Миграция на MongoDB, документная модель и валидация схем

---

## О проекте

В данной лабораторной работе выполнена миграция сущностей **посылок (parcels)** и **доставок (deliveries)** из PostgreSQL в **MongoDB 7**. Использована документоориентированная модель данных с обоснованным выбором между embedding и references. Реализованы CRUD операции с использованием MongoDB операторов (`$push`, `$addToSet`, `$set`), валидация схем через `$jsonSchema`, а также агрегационные pipelines для аналитики.

**Вариант задания:** №6 — Сервис доставки (Пользователь, Посылка, Доставка)

**Функционал:**
- 👤 Пользователи — PostgreSQL (оставлены)
- 📦 Посылки — MongoDB (с валидацией)
- 🚚 Доставки — MongoDB (с историей событий)
- 🔐 JWT-аутентификация

---

## Технологии

| Технология | Версия | Назначение |
|:---|:---:|:---|
| C++20 | - | Язык программирования |
| Yandex Userver | latest | Фреймворк |
| PostgreSQL | 16 | Пользователи |
| MongoDB | 7 | Посылки и доставки |
| JWT (jwt-cpp) | 0.7.2 | Аутентификация |
| Docker, docker-compose | - | Контейнеризация |
| OpenAPI 3.0 + Swagger UI | - | Документация API |
| pytest + requests | - | Тестирование |

---

## Быстрый старт

### Запуск

```bash
# Перейти в директорию ЛР-4
cd delivery-api-lr-4

# Сборка и запуск
docker-compose up --build

# Открыть Swagger UI
http://localhost:8080/api/v1/docs
```

### Заполнение тестовыми данными

```bash
# Для Linux/macOS
cat data.js | docker exec -i mongodb_delivery mongosh -u admin -p password

# Для Windows PowerShell
Get-Content data.js | docker exec -i mongodb_delivery mongosh -u admin -p password
```

---

## Документная модель MongoDB

### Коллекция `parcels` (посылки)

```javascript
{
    "_id": "UUID",                         // Уникальный идентификатор
    "sender_id": 1,                        // Ссылка на PostgreSQL users
    "weight": 2.5,                         // Вес в кг (≥ 0.01)
    "dimensions": "30x20x15",              // Формат: цифраxцифраxцифра
    "declared_value": 5000.0,              // Объявленная стоимость (≥ 0)
    "description": "Книги и документы",    // Описание (max 500 символов)
    "status": "created",                   // created/assigned/in_transit/delivered
    "created_at": ISODate("2026-05-21T10:00:00Z"),
    "features": ["fragile", "electronic"]  // Массив особенностей
}
```

### Коллекция `deliveries` (доставки)

```javascript
{
    "_id": "UUID",
    "parcel_id": "UUID",                   // Ссылка на parcels
    "sender_id": 1,                        // Ссылка на PostgreSQL users
    "receiver_id": 2,                      // Ссылка на PostgreSQL users
    "tracking_number": "DEL-1-ABC123",     // Уникальный формат: DEL-цифры-6букв
    "status": "pending",                   // pending/in_transit/delivered/cancelled
    "from_address": "Москва, Тверская 15", // ≥ 5 символов
    "to_address": "Санкт-Петербург, Невский 25",
    "created_at": ISODate("2026-05-21T10:00:00Z"),
    "delivered_at": null,                  // Дата доставки (если доставлена)
    "events": [                            // История статусов
        {
            "status": "pending",
            "timestamp": ISODate("2026-05-21T10:00:00Z"),
            "location": "Москва",
            "comment": "Заказ создан"
        }
    ]
}
```

---

## Обоснование выбора Embedded/References

| Связь | Решение | Обоснование |
|:---|:---|:---|
| `parcels.sender_id` → `users.id` | **Reference** | Пользователи в PostgreSQL, избежание дублирования |
| `deliveries.parcel_id` → `parcels._id` | **Reference** | Посылки изменяются независимо от доставок |
| `deliveries.events` | **Embedded** | История жестко привязана к доставке, нет отдельной сущности |
| `parcels.features` | **Embedded** | Массив строк, не требует отдельной коллекции |

---

## Валидация схем (JSON Schema)

### Валидатор для `parcels`

```javascript
{
    $jsonSchema: {
        bsonType: "object",
        required: ["sender_id", "weight", "dimensions", "declared_value", "status", "created_at"],
        properties: {
            weight: { bsonType: "double", minimum: 0.01 },
            dimensions: { pattern: "^\\d+x\\d+x\\d+$" },
            status: { enum: ["created", "assigned", "in_transit", "delivered"] },
            features: { bsonType: "array", items: { bsonType: "string" } }
        }
    }
}
```

### Валидатор для `deliveries`

```javascript
{
    $jsonSchema: {
        bsonType: "object",
        required: ["parcel_id", "sender_id", "receiver_id", "tracking_number", "status", "from_address", "to_address", "created_at", "events"],
        properties: {
            tracking_number: { pattern: "^DEL-\\d+-[A-Z0-9]{6}$" },
            status: { enum: ["pending", "in_transit", "delivered", "cancelled"] },
            events: { bsonType: "array", items: { required: ["status", "timestamp", "location"] } }
        }
    }
}
```

---

## MongoDB операции

### Create (вставка)

```javascript
// Создание посылки
db.parcels.insertOne({
    _id: uuid(),
    sender_id: 1,
    weight: 2.5,
    dimensions: "30x20x15",
    declared_value: 5000,
    status: "created",
    created_at: new Date(),
    features: []
});

// Создание доставки (автоматически обновляет статус посылки)
db.deliveries.insertOne({
    _id: uuid(),
    parcel_id: parcelId,
    sender_id: 1,
    receiver_id: 2,
    tracking_number: "DEL-1-ABC123",
    status: "pending",
    from_address: "Москва, Тверская 15",
    to_address: "СПб, Невский 25",
    created_at: new Date(),
    events: [{
        status: "pending",
        timestamp: new Date(),
        location: "Москва",
        comment: "Заказ создан"
    }]
});
```

### Read (поиск)

```javascript
// Поиск посылок отправителя
db.parcels.find({ sender_id: 1 }).sort({ created_at: -1 });

// Поиск доставки по трек-номеру
db.deliveries.findOne({ tracking_number: "DEL-1-ABC123" });

// Поиск с несколькими условиями
db.deliveries.find({
    $and: [
        { sender_id: 1 },
        { status: { $in: ["pending", "in_transit"] } }
    ]
});
```

### Update (обновление)

```javascript
// Добавление особенности к посылке ($addToSet)
db.parcels.updateOne(
    { _id: parcelId },
    { $addToSet: { features: "fragile" } }
);

// Обновление статуса доставки с добавлением события ($set + $push)
db.deliveries.updateOne(
    { _id: deliveryId },
    {
        $set: { status: "in_transit" },
        $push: {
            events: {
                status: "in_transit",
                timestamp: new Date(),
                location: "Сортировочный центр",
                comment: "Отправлено"
            }
        }
    }
);
```

### Delete (удаление)

```javascript
// Удаление посылки (только если нет связанных доставок)
db.parcels.deleteOne({ _id: parcelId });
```

---

## 📊 Агрегационные pipelines

### Статистика по статусам доставок

```javascript
db.deliveries.aggregate([
    { $group: {
        _id: "$status",
        count: { $sum: 1 },
        avg_delivery_time_hours: {
            $avg: {
                $cond: [
                    { $ne: ["$delivered_at", null] },
                    { $divide: [{ $subtract: ["$delivered_at", "$created_at"] }, 3600000] },
                    null
                ]
            }
        }
    } },
    { $project: { status: "$_id", count: 1, avg_delivery_time_hours: { $round: ["$avg_delivery_time_hours", 2] }, _id: 0 } }
]);
```

### Топ отправителей по количеству доставок

```javascript
db.deliveries.aggregate([
    { $group: {
        _id: "$sender_id",
        delivery_count: { $sum: 1 },
        completed_count: { $sum: { $cond: [{ $eq: ["$status", "delivered"] }, 1, 0] } }
    } },
    { $sort: { delivery_count: -1 } },
    { $limit: 5 }
]);
```

### Дневной объем доставок

```javascript
db.deliveries.aggregate([
    { $match: { created_at: { $gte: tenDaysAgo } } },
    { $group: {
        _id: { $dateToString: { format: "%Y-%m-%d", date: "$created_at" } },
        count: { $sum: 1 }
    } },
    { $sort: { _id: -1 } }
]);
```

---

## API Endpoints

Базовый URL: `http://localhost:8080/api/v1`

| Метод | Путь | Описание | Аутентификация |
|:---|:---|:---|:---:|
| `POST` | `/users` | Регистрация (PostgreSQL) | ❌ |
| `POST` | `/login` | Получение JWT | ❌ |
| `GET` | `/users` | Поиск пользователей | ❌ |
| `POST` | `/parcels` | Создание посылки (MongoDB) | ✅ |
| `GET` | `/parcels` | Посылки пользователя | ❌ |
| `POST` | `/deliveries` | Создание доставки (MongoDB) | ✅ |
| `GET` | `/deliveries` | Поиск доставок | ❌ |
| `GET` | `/docs` | Swagger UI | ❌ |
| `GET` | `/docs/openapi.yaml` | OpenAPI спецификация | ❌ |

---

## Примеры запросов

### 1. Создание посылки (MongoDB)

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
    "description": "Книги",
    "features": ["fragile"]
  }'
```

**Ответ (201 Created):**
```json
{
    "id": "550e8400-e29b-41d4-a716-446655440000",
    "sender_id": 1,
    "weight": 2.5,
    "dimensions": "30x20x15",
    "declared_value": 5000,
    "description": "Книги",
    "status": "created",
    "created_at": "2026-05-21T10:00:00Z",
    "features": ["fragile"]
}
```

### 2. Создание доставки

```bash
curl -X POST http://localhost:8080/api/v1/deliveries \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "parcel_id": "550e8400-e29b-41d4-a716-446655440000",
    "sender_id": 1,
    "receiver_id": 2,
    "from_address": "Москва, Тверская 15",
    "to_address": "СПб, Невский 25"
  }'
```

**Ответ (201 Created):**
```json
{
    "id": "660e8400-e29b-41d4-a716-446655440001",
    "parcel_id": "550e8400-e29b-41d4-a716-446655440000",
    "sender_id": 1,
    "receiver_id": 2,
    "tracking_number": "DEL-1-ABC123",
    "status": "pending",
    "from_address": "Москва, Тверская 15",
    "to_address": "СПб, Невский 25",
    "created_at": "2026-05-21T10:05:00Z",
    "delivered_at": null,
    "events": [...]
}
```

### 3. Поиск доставки по трек-номеру

```bash
curl "http://localhost:8080/api/v1/deliveries?tracking_number=DEL-1-ABC123"
```

---

## 📁 Структура проекта

```
delivery-api-lr-4/
├── src/
│   ├── components/
│   │   ├── auth_component.cpp/hpp
│   │   └── mongo_storage_component.cpp/hpp   # MongoDB операции
│   ├── handlers/
│   ├── jwt_auth/
│   ├── models/
│   └── main.cpp
├── configs/
│   ├── static_config.yaml
│   └── openapi.yaml
├── tests/
├── schema_design.md      # Обоснование документной модели
├── data.js               # Тестовые данные (10 посылок, 10 доставок)
├── queries.js            # MongoDB запросы (CRUD + операторы)
├── validation.js         # Валидация схем JSON Schema
├── aggregation.js        # Aggregation pipelines
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── README.md
```

---

## Тестирование

### Запуск тестов

```bash
pip install -r tests/requirements.txt
pytest tests/test_api.py -v
```

### Проверка валидации

```bash
# Попытка вставить невалидную посылку
docker exec -i mongodb_delivery mongosh -u admin -p password <<EOF
use delivery_db
db.parcels.insertOne({
    sender_id: "not_a_number",  # ❌ должно быть число
    weight: -1,                  # ❌ должно быть ≥ 0.01
    dimensions: "invalid",       # ❌ должно быть "30x20x15"
})
EOF
```

### Запуск агрегаций

```bash
cat aggregation.js | docker exec -i mongodb_delivery mongosh -u admin -p password
```

---

## Документация

- **Swagger UI:** `http://localhost:8080/api/v1/docs`
- **OpenAPI спецификация:** `http://localhost:8080/api/v1/docs/openapi.yaml`
- **Обоснование документной модели:** [schema_design.md](schema_design.md)

---
