# 📦 Delivery Service API — Лабораторная работа 5

> Кеширование через Redis и Rate Limiting

---

## О проекте

В данной лабораторной работе реализована стратегия повышения производительности и устойчивости системы к высоким нагрузкам. Внедрено **кеширование** через Redis для снижения нагрузки на базы данных и **ограничение частоты запросов (Rate Limiting)** для защиты API от злоупотреблений.

**Вариант задания:** №6 — Сервис доставки (Пользователь, Посылка, Доставка)

**Функционал:**
- 👤 Пользователи — PostgreSQL + кеш Redis
- 📦 Посылки — MongoDB + кеш Redis
- 🚚 Доставки — MongoDB + кеш Redis
- ⚡ Кеширование (Cache-Aside)
- 🚦 Rate Limiting (Token Bucket)
- 🔐 JWT-аутентификация

---

## Технологии

| Технология | Версия | Назначение |
|:---|:---:|:---|
| C++20 | - | Язык программирования |
| Yandex Userver | latest | Фреймворк |
| PostgreSQL | 16 | Пользователи |
| MongoDB | 7 | Посылки и доставки |
| Redis | 8 | Кеширование |
| JWT (jwt-cpp) | 0.7.2 | Аутентификация |
| Docker, docker-compose | - | Контейнеризация |
| OpenAPI 3.0 + Swagger UI | - | Документация API |
| pytest + requests | - | Тестирование |

---

## Быстрый старт

### Запуск

```bash
# Перейти в директорию ЛР-5
cd delivery-api-lr-5

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

## Кеширование (Caching)

### Стратегия: Cache-Aside (Lazy Loading)

```
1. Клиент → GET /deliveries?sender_id=1
2. API → Redis (проверка кеша)
   ├── HIT → возврат данных (быстрый ответ)
   └── MISS → MongoDB → сохранение в Redis → возврат данных
```

### Кешируемые эндпоинты

| Эндпоинт | Ключ кеша | TTL |
|:---|:---|:---:|
| `GET /deliveries?sender_id={id}` | `deliveries:sender:{id}:{from}:{to}` | 60 сек |
| `GET /deliveries?receiver_id={id}` | `deliveries:receiver:{id}:{from}:{to}` | 60 сек |
| `GET /deliveries?tracking_number={num}` | `delivery:tracking:{num}` | 300 сек |
| `GET /users?login={login}` | `user:login:{login}` | 300 сек |

### Инвалидация кеша

При создании новой доставки (`POST /deliveries`):

```cpp
cache_.InvalidateSenderDeliveriesCache(dto.sender_id);
cache_.InvalidateReceiverDeliveriesCache(dto.receiver_id);
```

### Пример работы кеша

```bash
# Первый запрос (MISS) — данные из MongoDB
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
# Время ответа: ~50 мс

# Второй запрос (HIT) — данные из Redis
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
# Время ответа: ~5 мс (ускорение в 10 раз)
```

---

## Rate Limiting

### Алгоритм: Token Bucket

| Параметр | Значение |
|:---|:---|
| Лимит | 100 запросов в минуту |
| Размер ведра | 100 токенов |
| Скорость пополнения | 1.67 токенов/сек |
| Защищенный эндпоинт | `GET /deliveries` |

### Заголовки ответа

| Заголовок | Описание |
|:---|:---|
| `X-RateLimit-Limit` | Максимальное количество запросов |
| `X-RateLimit-Remaining` | Оставшиеся запросы |
| `X-RateLimit-Reset` | Время сброса (Unix timestamp) |
| `Retry-After` | Время ожидания в секундах |

### Пример превышения лимита

```bash
# 101-й запрос за минуту
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

**Ответ (429 Too Many Requests):**
```json
{
    "code": "TOO_MANY_REQUESTS",
    "message": "Too many requests were made"
}
```

**Заголовки:**
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 0
X-RateLimit-Reset: 1737386400
Retry-After: 60
```

---

## 📊 Анализ производительности

### Влияние кеширования

| Метрика | Без кеша | С кешем | Улучшение |
|:---|:---:|:---:|:---:|
| Время отклика (P95) | ~50 мс | ~5 мс | **10x** |
| Нагрузка на MongoDB | 100% | ~20% | **5x** |
| Пропускная способность | 200 RPS | 1000+ RPS | **5x** |

### Cache Hit Rate (ожидаемый)

| Эндпоинт | Hit Rate |
|:---|:---:|
| `GET /deliveries?sender_id=` | 70-80% |
| `GET /deliveries?receiver_id=` | 70-80% |
| `GET /deliveries?tracking_number=` | 60-70% |
| `GET /users?login=` | 80-90% |

### Влияние Rate Limiting

- Защита от DDoS и Brute-Force атак
- Стабильность системы при пиковых нагрузках
- Предотвращение исчерпания ресурсов (CPU, RAM, Connections)

---

## API Endpoints

Базовый URL: `http://localhost:8080/api/v1`

| Метод | Путь | Описание | Кеш | Rate Limit |
|:---|:---|:---|:---:|:---:|
| `POST` | `/users` | Регистрация | ❌ | ❌ |
| `POST` | `/login` | Получение JWT | ❌ | ❌ |
| `GET` | `/users?login=` | Поиск по логину | ✅ | ❌ |
| `GET` | `/users?name_mask=` | Поиск по маске | ❌ | ❌ |
| `POST` | `/parcels` | Создание посылки | ❌ | ❌ |
| `GET` | `/parcels?user_id=` | Посылки пользователя | ❌ | ❌ |
| `POST` | `/deliveries` | Создание доставки | ❌ | ❌ |
| `GET` | `/deliveries` | Поиск доставок | ✅ | ✅ |

---

## Примеры запросов

### 1. Первый запрос (кеш MISS)

```bash
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

**Логи:**
```
[INFO] Cache MISS for key: deliveries:sender:1:1:10
[INFO] Querying MongoDB for sender_id=1
[INFO] Saving to Redis with TTL=60
[INFO] Response time: 52ms
```

### 2. Второй запрос (кеш HIT)

```bash
curl "http://localhost:8080/api/v1/deliveries?sender_id=1"
```

**Логи:**
```
[INFO] Cache HIT for key: deliveries:sender:1:1:10
[INFO] Response time: 4ms
```

### 3. Проверка Rate Limiting

```bash
# Быстро 100 запросов подряд
for i in {1..101}; do
    curl -s "http://localhost:8080/api/v1/deliveries?sender_id=1" | head -1
done
```

**Результат:** 100 успешных ответов, 101-й с ошибкой 429.

### 4. Создание доставки (инвалидация кеша)

```bash
TOKEN="ваш_jwt_токен"

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
```

---

## 📁 Структура проекта

```
delivery-api-lr-5/
├── src/
│   ├── components/
│   │   ├── auth_component.cpp/hpp
│   │   ├── mongo_storage_component.cpp/hpp
│   │   ├── postgres_storage_component.cpp/hpp
│   │   ├── rate_limiter_component.cpp/hpp   # Rate Limiting
│   │   └── redis_cache_component.cpp/hpp    # Redis кеш
│   ├── handlers/
│   ├── jwt_auth/
│   ├── models/
│   └── main.cpp
├── configs/
│   ├── static_config.yaml
│   ├── openapi.yaml
│   └── secdist.json          # Redis настройки
├── tests/
├── performance_design.md     # Стратегия кеширования и Rate Limiting
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

### Проверка кеша в Redis

```bash
# Подключиться к Redis
docker exec -it redis_delivery redis-cli

# Посмотреть все ключи
KEYS *

# Посмотреть TTL
TTL "deliveries:sender:1:1:10"

# Удалить кеш по ключу
DEL "deliveries:sender:1:1:10"
```

### Проверка метрик

```bash
# Статистика Redis
INFO stats
# keypace_hits, keypace_misses
```

---

## Документация

- **Swagger UI:** `http://localhost:8080/api/v1/docs`
- **OpenAPI спецификация:** `http://localhost:8080/api/v1/docs/openapi.yaml`
- **Дизайн производительности:** [performance_design.md](performance_design.md)

---
