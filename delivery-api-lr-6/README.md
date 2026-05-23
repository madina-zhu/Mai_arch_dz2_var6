# 📦 Delivery Service API — Лабораторная работа 6

> Event-Driven архитектура, RabbitMQ и CQRS

---

## О проекте

В данной лабораторной работе реализована **событийно-ориентированная архитектура (Event-Driven Architecture)** с использованием брокера сообщений **RabbitMQ**. Применен паттерн **CQRS (Command Query Responsibility Segregation)** для разделения операций записи и чтения. События синхронизируют Write Database (PostgreSQL + MongoDB) и Read Database (in-memory кэш).

**Вариант задания:** №6 — Сервис доставки (Пользователь, Посылка, Доставка)

**Функционал:**
- 👤 Пользователи — PostgreSQL + события
- 📦 Посылки — MongoDB + события
- 🚚 Доставки — MongoDB + события
- 📨 Event Producer / Consumer (RabbitMQ)
- 🔀 CQRS (разделение Command и Query)
- ⚡ Кеширование (Redis)
- 🚦 Rate Limiting
- 🔐 JWT-аутентификация

---

## Технологии

| Технология | Версия | Назначение |
|:---|:---:|:---|
| C++20 | - | Язык программирования |
| Yandex Userver | latest | Фреймворк |
| PostgreSQL | 16 | Write Database (пользователи) |
| MongoDB | 7 | Write Database (посылки, доставки) |
| Redis | 8 | Кеширование |
| RabbitMQ | 3.13 | Брокер сообщений |
| JWT (jwt-cpp) | 0.7.2 | Аутентификация |
| Docker, docker-compose | - | Контейнеризация |
| OpenAPI 3.0 + Swagger UI | - | Документация API |
| pytest + requests | - | Тестирование |

---

## Быстрый старт

### Запуск

```bash
# Перейти в директорию ЛР-6
cd delivery-api-lr-6

# Сборка и запуск
docker-compose up --build

# Открыть Swagger UI
http://localhost:8080/api/v1/docs
```

### Заполнение тестовыми данными

```bash
# PostgreSQL (пользователи)
cat data.sql | docker exec -i postgres_delivery psql -U delivery_user -d delivery_db

# MongoDB (посылки и доставки)
cat data.js | docker exec -i mongodb_delivery mongosh -u admin -p password
```

---

## Event-Driven архитектура

### Компоненты системы

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Клиент (HTTP)                                   │
└─────────────────────────────────┬───────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         API Gateway (Userver)                               │
│  ┌─────────────────────┐    ┌─────────────────────┐    ┌─────────────────┐ │
│  │   Command Handlers  │    │    Query Handlers   │    │  Rate Limiter   │ │
│  │   POST /parcels     │    │    GET /deliveries  │    │    Redis Cache  │ │
│  │   POST /deliveries  │    │    GET /users       │    │                 │ │
│  └──────────┬──────────┘    └──────────┬──────────┘    └─────────────────┘ │
│             │                          │                                    │
│             ▼                          ▼                                    │
│  ┌─────────────────────┐    ┌─────────────────────┐                        │
│  │   Write Database    │    │    Read Database    │                        │
│  │   (PostgreSQL +     │◄───│    (In-memory       │                        │
│  │    MongoDB)         │    │     + Redis)        │                        │
│  └──────────┬──────────┘    └─────────────────────┘                        │
│             │                                                               │
│             ▼                                                               │
│  ┌─────────────────────┐                                                    │
│  │  Event Producer     │                                                    │
│  │  (RabbitMQ Client)  │                                                    │
│  └──────────┬──────────┘                                                    │
└─────────────┼───────────────────────────────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         RabbitMQ Broker                                     │
│                    Exchange: "delivery_events"                              │
│                    Queue: "delivery_events_queue"                           │
│                    Routing Key: "#" (fan-out)                               │
└─────────────────────────────────┬───────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Event Consumer                                      │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  user.registered    →  Обновление Read Database (пользователь)      │   │
│  │  parcel.created     →  Обновление Read Database (посылка)           │   │
│  │  delivery.created   →  Обновление Read Database (доставка)          │   │
│  │  delivery.status_changed → Обновление статуса в Read Database       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Каталог событий (Event Catalog)

### 1. UserRegistered

| Параметр | Значение |
|:---|:---|
| **Название** | `user.registered` |
| **Производитель** | `CreateUserHandler` |
| **Потребители** | Read Database, Notification (заглушка) |
| **Гарантия** | At-least-once |

**Payload:**
```json
{
    "user_id": 123,
    "login": "john_doe",
    "email": "john@example.com",
    "first_name": "John",
    "last_name": "Doe"
}
```

### 2. ParcelCreated

| Параметр | Значение |
|:---|:---|
| **Название** | `parcel.created` |
| **Производитель** | `CreateParcelHandler` |
| **Потребители** | Read Database |
| **Гарантия** | At-least-once |

**Payload:**
```json
{
    "id": "550e8400-e29b-41d4-a716-446655440000",
    "sender_id": 1,
    "weight": 2.5,
    "dimensions": "30x20x15",
    "declared_value": 5000,
    "description": "Книги",
    "status": "created"
}
```

### 3. DeliveryCreated

| Параметр | Значение |
|:---|:---|
| **Название** | `delivery.created` |
| **Производитель** | `CreateDeliveryHandler` |
| **Потребители** | Read Database, Cache Invalidator |
| **Гарантия** | At-least-once |

**Payload:**
```json
{
    "id": "660e8400-e29b-41d4-a716-446655440001",
    "parcel_id": "550e8400-e29b-41d4-a716-446655440000",
    "sender_id": 1,
    "receiver_id": 2,
    "tracking_number": "DEL-1-ABC123",
    "status": "pending",
    "from_address": "Москва, Тверская 15",
    "to_address": "СПб, Невский 25"
}
```

### 4. DeliveryStatusChanged

| Параметр | Значение |
|:---|:---|
| **Название** | `delivery.status_changed` |
| **Производитель** | Обновление статуса доставки |
| **Потребители** | Read Database, Notification (заглушка) |
| **Гарантия** | At-least-once |

**Payload:**
```json
{
    "delivery_id": "660e8400-e29b-41d4-a716-446655440001",
    "old_status": "pending",
    "new_status": "in_transit"
}
```

---

## CQRS (Command Query Responsibility Segregation)

### Write Model (Команды)

| Операция | Хранилище | Генерирует событие |
|:---|:---|:---|
| `POST /users` | PostgreSQL | `user.registered` |
| `POST /parcels` | MongoDB | `parcel.created` |
| `POST /deliveries` | MongoDB | `delivery.created` |
| Обновление статуса | MongoDB | `delivery.status_changed` |

### Read Model (Запросы)

| Операция | Хранилище | Источник данных |
|:---|:---|:---|
| `GET /users?login=` | Read DB (in-memory) | События из RabbitMQ |
| `GET /parcels?user_id=` | Read DB (in-memory) | События из RabbitMQ |
| `GET /deliveries` | Redis Cache → Read DB | Кеш + события |

### Синхронизация через события

```
1. POST /parcels → Write DB (MongoDB)
2. Producer → RabbitMQ (parcel.created)
3. Consumer ← RabbitMQ → обновление Read DB
4. GET /parcels → Read DB (быстрый ответ)
```

---

## Компоненты реализации

### EventProducer (delivery_event_producer.hpp/cpp)

```cpp
class DeliveryEventProducer {
public:
    void PublishUserRegistered(const UserCreateRequest& user, int64_t user_id);
    void PublishParcelCreated(const std::string& id, const ParcelCreateRequest& parcel);
    void PublishDeliveryCreated(const DeliveryResponse& delivery);
    void PublishDeliveryStatusChanged(const std::string& delivery_id,
                                      const std::string& old_status,
                                      const std::string& new_status);
};
```

### EventConsumer (delivery_event_consumer.hpp/cpp)

```cpp
class DeliveryEventConsumer {
protected:
    void Process(std::string message) override;
    
private:
    void HandleUserRegistered(const userver::formats::json::Value& payload);
    void HandleParcelCreated(const userver::formats::json::Value& payload);
    void HandleDeliveryCreated(const userver::formats::json::Value& payload);
    void HandleDeliveryStatusChanged(const userver::formats::json::Value& payload);
};
```

### ReadDatabaseComponent (in-memory)

```cpp
class ReadDatabaseComponent {
public:
    // Обновление (вызывается consumer'ом)
    void OnUserCreated(int64_t user_id, const UserCreateRequest& user, const std::string& password_hash);
    void OnParcelCreated(const std::string& parcel_id, const ParcelCreateRequest& parcel);
    void OnDeliveryCreated(const DeliveryResponse& delivery);
    void OnDeliveryStatusChanged(const std::string& delivery_id, const std::string& new_status);
    
    // Чтение (для API запросов)
    std::optional<UserResponse> GetUserByLogin(const std::string& login, int from, int to);
    std::vector<DeliveryResponse> GetDeliveriesBySender(int64_t sender_id, int to, int from);
    std::optional<DeliveryResponse> GetDeliveryByTrackingNumber(const std::string& tracking_number);
};
```

---

## API Endpoints

Базовый URL: `http://localhost:8080/api/v1`

| Метод | Путь | Описание | Write/Read |
|:---|:---|:---|:---:|
| `POST` | `/users` | Регистрация | Write (PostgreSQL) |
| `POST` | `/login` | Получение JWT | Read (PostgreSQL) |
| `GET` | `/users` | Поиск пользователей | Read (in-memory) |
| `POST` | `/parcels` | Создание посылки | Write (MongoDB) |
| `GET` | `/parcels` | Посылки пользователя | Read (in-memory) |
| `POST` | `/deliveries` | Создание доставки | Write (MongoDB) |
| `GET` | `/deliveries` | Поиск доставок | Read (Redis + in-memory) |
| `GET` | `/docs` | Swagger UI | - |
| `GET` | `/docs/openapi.yaml` | OpenAPI | - |

---

## Примеры запросов

### 1. Регистрация пользователя (генерирует событие)

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

**Логи Producer:**
```
[INFO] Event published: user.registered [user.registered]
```

**Логи Consumer:**
```
[INFO] Consumed event: user.registered
[INFO] User registered event: john_doe
[INFO] ReadDB: User created: id=1000, login=john_doe
```

### 2. Создание посылки (генерирует событие)

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

**Логи:**
```
[INFO] Event published: parcel.created [parcel.created]
[INFO] Consumed event: parcel.created
[INFO] Parcel created event: id=550e..., sender=1
[INFO] ReadDB: Parcel created: id=550e..., sender=1
```

### 3. Создание доставки (генерирует событие + инвалидация кеша)

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

**Логи:**
```
[INFO] Invalidating cache for sender_id=1
[INFO] Invalidating cache for receiver_id=2
[INFO] Event published: delivery.created [delivery.created]
[INFO] Consumed event: delivery.created
[INFO] Delivery created event: id=660e..., tracking=DEL-1-ABC123
[INFO] ReadDB: Delivery created: id=660e..., tracking=DEL-1-ABC123
```

### 4. Поиск доставок (читает из Read Database + кеш)

```bash
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

**Логи:**
```
[INFO] Cache HIT for key: deliveries:sender:1:1:10
[INFO] Response from Redis cache
```

---

## 📁 Структура проекта

```
delivery-api-lr-6/
├── src/
│   ├── components/
│   │   ├── auth_component.cpp/hpp
│   │   ├── delivery_event_consumer.cpp/hpp   # Consumer RabbitMQ
│   │   ├── delivery_event_producer.cpp/hpp   # Producer RabbitMQ
│   │   ├── mongo_storage_component.cpp/hpp   # Write DB (MongoDB)
│   │   ├── postgres_storage_component.cpp/hpp # Write DB (PostgreSQL)
│   │   ├── rate_limiter_component.cpp/hpp    # Rate Limiting
│   │   ├── read_database_component.cpp/hpp   # Read DB (in-memory)
│   │   └── redis_cache_component.cpp/hpp     # Redis кеш
│   ├── handlers/
│   ├── jwt_auth/
│   ├── models/
│   └── main.cpp
├── configs/
│   ├── static_config.yaml
│   ├── openapi.yaml
│   └── secdist.json              # Redis + RabbitMQ настройки
├── tests/
├── event_driven_design.md        # Описание Event-Driven архитектуры
├── event_catalog.md              # Каталог событий
├── performance_design.md
├── schema_design.md
├── schema.sql
├── data.js
├── data.sql
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

### Проверка RabbitMQ

```bash
# Открыть RabbitMQ Management UI
http://localhost:15672
# Логин: guest, Пароль: guest

# Просмотр очередей
rabbitmqadmin list queues

# Просмотр сообщений в очереди
rabbitmqadmin get queue=delivery_events_queue
```

### Проверка состояния Read Database

```bash
# Read Database — in-memory (логи при запуске)
docker logs delivery_api | grep "ReadDB"
```

---

## Документация

- **Swagger UI:** `http://localhost:8080/api/v1/docs`
- **OpenAPI спецификация:** `http://localhost:8080/api/v1/docs/openapi.yaml`
- **Event-Driven архитектура:** [event_driven_design.md](event_driven_design.md)
- **Каталог событий:** [event_catalog.md](event_catalog.md)

---
