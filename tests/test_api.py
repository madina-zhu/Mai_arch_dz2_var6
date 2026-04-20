"""
Delivery Service API — Комплексные тесты
Покрывает все успешные сценарии и ошибки согласно OpenAPI спецификации.

Запуск:
    pytest tests/test_api.py -v
    pytest tests/test_api.py -v --tb=short  # Краткий вывод ошибок
"""

import pytest
import requests
import uuid
import time
from typing import Dict, Any
from datetime import datetime, timezone

# =============================================================================
# CONFIGURATION
# =============================================================================

BASE_URL = "http://localhost:8080/api/v1"
TIMEOUT = 10  # seconds

BASE_TEST_USER = {
    "login": "test_user",
    "password": "password123",
    "first_name": "Test",
    "last_name": "User",
    "email": "test@example.com"
}

BASE_TEST_PARCEL = {
    "title": "Тестовая посылка",
    "description": "Описание для тестов",
    "type": "standard",
    "from_city": "Moscow",
    "to_city": "Saint Petersburg",
    "address": {
        "street": "Test Street",
        "house": "1",
        "flat": "1"
    },
    "weight": 2.5,
    "price": 500,
    "details": {
        "is_fragile": False
    },
    "features": ["tracking"],
    "status": "pending"
}

BASE_TEST_DELIVERY = {
    "pickup_time": "2026-03-28T07:45:11.337294+00:00",
    "estimated_delivery": "2026-03-30T18:00:00+00:00",
    "courier_id": 101
}


# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def normalize(dt_str: str) -> datetime:
    """Нормализует строку даты в datetime"""
    return datetime.fromisoformat(dt_str.replace("Z", "+00:00")).astimezone(timezone.utc)


def generate_unique_string(prefix: str = "") -> str:
    """Генерирует уникальную строку для тестовых данных"""
    return f"{prefix}_{uuid.uuid4().hex[:8]}"


def create_test_user_data() -> Dict[str, Any]:
    """Создает уникальные данные тестового пользователя"""
    unique_id = uuid.uuid4().hex[:8]
    return {
        "login": f"user_{unique_id}",
        "password": "password123",
        "first_name": "Test",
        "last_name": "User",
        "email": f"test_{unique_id}@example.com"
    }


def create_test_parcel_data(sender_id: int, recipient_id: int) -> Dict[str, Any]:
    """Создает данные тестовой посылки"""
    unique_id = uuid.uuid4().hex[:8]
    data = BASE_TEST_PARCEL.copy()
    data["from_city"] = f"Test City {unique_id}"
    data["sender_id"] = sender_id
    data["recipient_id"] = recipient_id
    data["title"] = f"{BASE_TEST_PARCEL['title']}_{unique_id}"
    return data


def create_test_delivery_data(parcel_id: str, recipient_id: int, courier_id: int = 101) -> Dict[str, Any]:
    """Создает данные тестовой доставки"""
    data = BASE_TEST_DELIVERY.copy()
    data["parcel_id"] = parcel_id
    data["recipient_id"] = recipient_id
    data["courier_id"] = courier_id
    return data


def assert_error_response(response: requests.Response, expected_code: str):
    """Проверка формата ответа об ошибке"""
    
    assert response.status_code in [400, 401, 403, 404, 409, 422], \
        f"Expected error status code, got {response.status_code}"
    
    if response.headers.get("Content-Type", "").startswith("application/json"):
        data = response.json()
        assert "code" in data, "Error response missing 'code' field"
        assert "message" in data, "Error response missing 'message' field"
        assert data["code"] == expected_code, f"Expected code '{expected_code}', got '{data['code']}'"
    else:
        assert len(response.text) > 0 or len(response.content) > 0


def assert_success_response(response: requests.Response, expected_status: int):
    """Проверка успешного ответа"""
    assert response.status_code == expected_status, \
        f"Expected {expected_status}, got {response.status_code}. Body: {response.text}"
    
    if response.headers.get("Content-Type", "").startswith("application/json"):
        data = response.json()
        assert data is not None
    else:
        assert len(response.text) > 0 or len(response.content) > 0


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture(scope="session")
def api_base_url() -> str:
    """Базовый URL API"""
    return BASE_URL


@pytest.fixture(scope="function")
def session() -> requests.Session:
    """Сессия requests с автоматическими заголовками"""
    session = requests.Session()
    session.headers.update({"Content-Type": "application/json"})
    yield session
    session.close()


@pytest.fixture(scope="function")
def registered_user(session: requests.Session, api_base_url: str) -> Dict[str, Any]:
    """Фикстура: регистрирует и возвращает данные пользователя"""
    user_data = create_test_user_data()
    
    response = session.post(
        f"{api_base_url}/users",
        json=user_data,
        timeout=TIMEOUT
    )
    
    assert response.status_code == 201, f"Registration failed: {response.text}"
    user_id = response.json()["id"]
    
    return {
        "id": user_id,
        "login": user_data["login"],
        "password": user_data["password"],
        "data": user_data
    }


@pytest.fixture(scope="function")
def second_user(session: requests.Session, api_base_url: str) -> Dict[str, Any]:
    """Фикстура: регистрирует второго пользователя (получателя)"""
    user_data = create_test_user_data()
    
    response = session.post(
        f"{api_base_url}/users",
        json=user_data,
        timeout=TIMEOUT
    )
    
    assert response.status_code == 201, f"Registration failed: {response.text}"
    user_id = response.json()["id"]
    
    return {
        "id": user_id,
        "login": user_data["login"],
        "password": user_data["password"],
        "data": user_data
    }


@pytest.fixture(scope="function")
def auth_token(session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]) -> str:
    """Фикстура: логинится и возвращает токен"""
    response = session.post(
        f"{api_base_url}/login",
        json={
            "login": registered_user["login"],
            "password": registered_user["password"]
        },
        timeout=TIMEOUT
    )
    
    assert response.status_code == 200, f"Login failed: {response.text}"
    return response.json()["access_token"]


@pytest.fixture(scope="function")
def auth_headers(auth_token: str) -> dict:
    """Заголовки с авторизацией"""
    return {"Authorization": f"Bearer {auth_token}"}


@pytest.fixture(scope="function")
def test_user_with_token(session: requests.Session, api_base_url: str) -> Dict[str, Any]:
    """Фикстура: возвращает полные данные пользователя с токеном"""
    user_data = create_test_user_data()
    
    response = session.post(
        f"{api_base_url}/users",
        json=user_data,
        timeout=TIMEOUT
    )
    assert response.status_code == 201
    user_id = response.json()["id"]
    
    response = session.post(
        f"{api_base_url}/login",
        json={"login": user_data["login"], "password": user_data["password"]},
        timeout=TIMEOUT
    )
    assert response.status_code == 200
    token = response.json()["access_token"]
    
    return {
        "id": user_id,
        "login": user_data["login"],
        "password": user_data["password"],
        "token": token,
        "headers": {"Authorization": f"Bearer {token}"}
    }


@pytest.fixture(scope="function")
def created_parcel(
    session: requests.Session,
    api_base_url: str,
    test_user_with_token: Dict[str, Any],
    second_user: Dict[str, Any]
) -> dict:
    """Фикстура: создаёт тестовую посылку"""
    parcel_data = create_test_parcel_data(test_user_with_token["id"], second_user["id"])
    
    response = session.post(
        f"{api_base_url}/parcels",
        json=parcel_data,
        headers=test_user_with_token["headers"],
        timeout=TIMEOUT
    )
    
    assert response.status_code == 201, f"Parcel creation failed: {response.text}"
    return response.json()


@pytest.fixture(scope="function")
def created_delivery(
    session: requests.Session,
    api_base_url: str,
    test_user_with_token: Dict[str, Any],
    created_parcel: dict,
    second_user: Dict[str, Any]
) -> dict:
    """Фикстура: создаёт тестовую доставку"""
    delivery_data = create_test_delivery_data(
        created_parcel["id"],
        second_user["id"],
        courier_id=101
    )
    
    response = session.post(
        f"{api_base_url}/deliveries",
        json=delivery_data,
        headers=test_user_with_token["headers"],
        timeout=TIMEOUT
    )
    
    assert response.status_code == 201, f"Delivery creation failed: {response.text}"
    return response.json()


# =============================================================================
# TESTS: AUTH ENDPOINTS
# =============================================================================

class TestAuthEndpoints:
    """Тесты эндпоинта аутентификации: /login"""
    
    def test_login_success(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Успешный логин с получением токена"""
        response = session.post(
            f"{api_base_url}/login",
            json={
                "login": registered_user["login"],
                "password": registered_user["password"]
            },
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert "access_token" in data
        assert data.get("token_type") == "Bearer"
        assert data.get("login") == registered_user["login"]
        assert isinstance(data.get("user_id"), int)
    
    def test_login_wrong_password(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Логин с неправильным паролем -> 401"""
        response = session.post(
            f"{api_base_url}/login",
            json={"login": registered_user["login"], "password": "wrong_password"},
            timeout=TIMEOUT
        )
        
        assert response.status_code == 401
        assert_error_response(response, "UNAUTHORIZED")
    
    def test_login_nonexistent_user(self, session: requests.Session, api_base_url: str):
        """Логин несуществующего пользователя -> 401"""
        response = session.post(
            f"{api_base_url}/login",
            json={"login": "nonexistent_user_12345", "password": "any_password"},
            timeout=TIMEOUT
        )
        
        assert response.status_code == 401
        assert_error_response(response, "UNAUTHORIZED")
    
    def test_login_missing_credentials(self, session: requests.Session, api_base_url: str):
        """Логин без обязательных полей -> 400"""
        response = session.post(
            f"{api_base_url}/login",
            json={"login": "test"},
            timeout=TIMEOUT
        )
        
        assert response.status_code == 400
        assert_error_response(response, "VALIDATION_ERROR")


# =============================================================================
# TESTS: USER ENDPOINTS
# =============================================================================

class TestUserEndpoints:
    """Тесты эндпоинтов пользователей: /users"""
    
    def test_create_user_success(self, session: requests.Session, api_base_url: str):
        """Успешная регистрация нового пользователя"""
        user_data = create_test_user_data()
        
        response = session.post(
            f"{api_base_url}/users",
            json=user_data,
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 201)
        data = response.json()
        assert data["login"] == user_data["login"]
        assert data["email"] == user_data["email"]
        assert "id" in data
        assert "created_at" in data
    
    def test_create_user_missing_required_fields(self, session: requests.Session, api_base_url: str):
        """Регистрация без обязательных полей -> 400"""
        invalid_data = {"login": "test"}
        
        response = session.post(
            f"{api_base_url}/users",
            json=invalid_data,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 400
        assert_error_response(response, "VALIDATION_ERROR")
    
    def test_create_user_login_already_exists(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Повторная регистрация с существующим логином -> 409"""
        user_data = {
            "login": registered_user["login"],
            "password": "password123",
            "first_name": "Another",
            "last_name": "User",
            "email": "another@example.com"
        }
        
        response = session.post(
            f"{api_base_url}/users",
            json=user_data,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 409
        assert_error_response(response, "CONFLICT")
    
    def test_create_user_email_already_exists(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Повторная регистрация с существующим email -> 409"""
        user_data = {
            "login": "AnotherLogin",
            "password": "password123",
            "first_name": "Another",
            "last_name": "User",
            "email": registered_user["data"]["email"]
        }
        
        response = session.post(
            f"{api_base_url}/users",
            json=user_data,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 409
        assert_error_response(response, "CONFLICT")
    
    def test_search_users_by_login_success(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Поиск пользователя по точному логину -> 200"""
        response = session.get(
            f"{api_base_url}/users",
            params={
                "from": 1,
                "to": 10,
                "login": registered_user["login"]
            },
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1
        assert any(u["login"] == registered_user["login"] for u in data)
    
    def test_search_users_by_login_not_found(self, session: requests.Session, api_base_url: str):
        """Поиск несуществующего пользователя по логину -> 404"""
        response = session.get(
            f"{api_base_url}/users",
            params={
                "from": 1,
                "to": 10,
                "login": "nonexistent_login_xyz"
            },
            timeout=TIMEOUT
        )
        
        assert response.status_code == 404
        assert_error_response(response, "NOT_FOUND")
    
    def test_search_users_by_name_mask_success(self, session: requests.Session, api_base_url: str, registered_user: Dict[str, Any]):
        """Поиск пользователя по маске имени -> 200"""
        name_mask = registered_user["data"]["first_name"][:3] + "*"
        
        response = session.get(
            f"{api_base_url}/users",
            params={
                "from": 1,
                "to": 10,
                "name_mask": name_mask
            },
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
    
    def test_search_users_no_query_params(self, session: requests.Session, api_base_url: str):
        """Поиск без параметров -> 400"""
        response = session.get(f"{api_base_url}/users", timeout=TIMEOUT)
        
        assert response.status_code == 400
        assert_error_response(response, "BAD_REQUEST")


# =============================================================================
# TESTS: PARCEL ENDPOINTS
# =============================================================================

class TestParcelEndpoints:
    """Тесты эндпоинтов посылок: /parcels"""
    
    def test_create_parcel_with_auth_success(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], second_user: Dict[str, Any]):
        """Создание посылки с аутентификацией -> 201"""
        parcel_data = create_test_parcel_data(test_user_with_token["id"], second_user["id"])
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        
        assert response.status_code == 201, f"Expected 201, got {response.status_code}"
        
        if response.headers.get("Content-Type", "").startswith("application/json"):
            data = response.json()
            assert data["title"] == parcel_data["title"]
            assert data["from_city"] == parcel_data["from_city"]
            assert data["to_city"] == parcel_data["to_city"]
            assert "id" in data
            assert "created_at" in data
    
    def test_create_parcel_without_auth(self, session: requests.Session, api_base_url: str):
        """Создание посылки без токена -> 401"""
        parcel_data = create_test_parcel_data(sender_id=1, recipient_id=2)
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 401, f"Expected 401, got {response.status_code}"
    
    def test_create_parcel_invalid_sender_id(self, session: requests.Session, api_base_url: str, auth_headers: dict, second_user: Dict[str, Any]):
        """Создание посылки с несуществующим sender_id -> 404"""
        parcel_data = create_test_parcel_data(sender_id=999999, recipient_id=second_user["id"])
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=auth_headers,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 404
        assert_error_response(response, "NOT_FOUND")
    
    def test_create_parcel_missing_required_fields(self, session: requests.Session, api_base_url: str, auth_headers: dict):
        """Создание посылки без обязательных полей -> 400"""
        invalid_parcel = {"title": "Incomplete"}
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=invalid_parcel,
            headers=auth_headers,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 400
    
    def test_search_parcels_by_from_city_success(self, session: requests.Session, api_base_url: str, created_parcel: dict):
        """Поиск посылок по городу отправления -> 200"""
        response = session.get(
            f"{api_base_url}/parcels",
            params={
                "from": 1,
                "to": 10,
                "from_city": created_parcel["from_city"]
            },
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1
    
    def test_search_parcels_by_sender_id_success(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], created_parcel: dict):
        """Получение посылок пользователя по отправителю -> 200"""
        response = session.get(
            f"{api_base_url}/parcels",
            params={"sender_id": test_user_with_token["id"]},
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1
        assert any(p["id"] == created_parcel["id"] for p in data)
    
    def test_search_parcels_by_weight_range_success(self, session: requests.Session, api_base_url: str):
        """Поиск посылок по диапазону веса -> 200"""
        response = session.get(
            f"{api_base_url}/parcels",
            params={
                "from": 1,
                "to": 10,
                "min_weight": 1.0,
                "max_weight": 10.0
            },
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
    
    def test_search_parcels_no_params(self, session: requests.Session, api_base_url: str):
        """Поиск без параметров -> 400"""
        response = session.get(f"{api_base_url}/parcels", timeout=TIMEOUT)
        
        assert response.status_code == 400
        assert_error_response(response, "BAD_REQUEST")
    
    def test_search_parcels_invalid_weight_format(self, session: requests.Session, api_base_url: str):
        """Поиск с невалидным форматом веса -> 400"""
        response = session.get(
            f"{api_base_url}/parcels",
            params={
                "from": 1,
                "to": 10,
                "min_weight": "not_a_number",
                "max_weight": "10.0"
            },
            timeout=TIMEOUT
        )
        
        assert response.status_code == 400
        assert_error_response(response, "VALIDATION_ERROR")


# =============================================================================
# TESTS: DELIVERY ENDPOINTS
# =============================================================================

class TestDeliveryEndpoints:
    """Тесты эндпоинтов доставок: /deliveries"""
    
    def test_create_delivery_with_auth_success(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], created_parcel: dict, second_user: Dict[str, Any]):
        """Создание доставки с аутентификацией -> 201"""
        delivery_data = create_test_delivery_data(
            created_parcel["id"],
            second_user["id"],
            courier_id=101
        )
        
        response = session.post(
            f"{api_base_url}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        
        assert response.status_code == 201, f"Expected 201, got {response.status_code}"
        
        if response.headers.get("Content-Type", "").startswith("application/json"):
            data = response.json()
            assert data["parcel_id"] == created_parcel["id"]
            assert normalize(data["pickup_time"]) == normalize(delivery_data["pickup_time"])
    
    def test_create_delivery_without_auth(self, session: requests.Session, api_base_url: str, created_parcel: dict, second_user: Dict[str, Any]):
        """Создание доставки без токена -> 401"""
        delivery_data = create_test_delivery_data(
            created_parcel["id"],
            second_user["id"]
        )
        
        response = session.post(
            f"{api_base_url}/deliveries",
            json=delivery_data,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 401, f"Expected 401, got {response.status_code}"
    
    def test_create_delivery_nonexistent_parcel(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], second_user: Dict[str, Any]):
        """Создание доставки для несуществующей посылки -> 404"""
        delivery_data = create_test_delivery_data(
            "nonexistent-parcel-id",
            second_user["id"]
        )
        
        response = session.post(
            f"{api_base_url}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        
        assert response.status_code == 404
        assert_error_response(response, "NOT_FOUND")
    
    def test_create_delivery_missing_required_fields(self, session: requests.Session, api_base_url: str, auth_headers: dict, created_parcel: dict):
        """Создание доставки без обязательных полей -> 400"""
        invalid_delivery = {"parcel_id": created_parcel["id"]}
        
        response = session.post(
            f"{api_base_url}/deliveries",
            json=invalid_delivery,
            headers=auth_headers,
            timeout=TIMEOUT
        )
        
        assert response.status_code == 400
    
    def test_get_deliveries_by_recipient_id_success(self, session: requests.Session, api_base_url: str, created_delivery: dict):
        """Получение доставок по получателю -> 200"""
        response = session.get(
            f"{api_base_url}/deliveries",
            params={"recipient_id": created_delivery["recipient_id"]},
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1
    
    def test_get_deliveries_by_sender_id_success(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], created_parcel: dict):
        """Получение доставок по отправителю -> 200"""
        response = session.get(
            f"{api_base_url}/deliveries",
            params={"sender_id": test_user_with_token["id"]},
            timeout=TIMEOUT
        )
        
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
    
    def test_get_deliveries_no_params(self, session: requests.Session, api_base_url: str):
        """Получение доставок без параметров -> 400"""
        response = session.get(f"{api_base_url}/deliveries", timeout=TIMEOUT)
        
        assert response.status_code == 400
        assert_error_response(response, "BAD_REQUEST")


# =============================================================================
# TESTS: DOCS ENDPOINTS
# =============================================================================

class TestDocsEndpoints:
    """Тесты эндпоинтов документации: /docs, /docs/openapi.yaml"""
    
    def test_get_openapi_spec_success(self, session: requests.Session, api_base_url: str):
        """Получение OpenAPI спецификации -> 200"""
        response = session.get(f"{api_base_url}/docs/openapi.yaml", timeout=TIMEOUT)
        
        assert response.status_code == 200
        assert "openapi:" in response.text or "swagger:" in response.text
    
    def test_get_swagger_ui_success(self, session: requests.Session, api_base_url: str):
        """Получение Swagger UI -> 200"""
        response = session.get(f"{api_base_url}/docs", timeout=TIMEOUT)
        
        assert response.status_code == 200
        assert response.headers["Content-Type"].startswith("text/html")
        assert "<!DOCTYPE html>" in response.text or "<html" in response.text


# =============================================================================
# TESTS: EDGE CASES & INTEGRATION
# =============================================================================

class TestEdgeCases:
    """Пограничные случаи и интеграционные тесты"""
    
    def test_full_workflow(self, session: requests.Session, api_base_url: str):
        """Полный сценарий: регистрация -> логин -> создание посылки -> создание доставки"""
        # Регистрация отправителя
        sender_data = create_test_user_data()
        response = session.post(f"{api_base_url}/users", json=sender_data, timeout=TIMEOUT)
        assert response.status_code == 201
        sender_id = response.json()["id"]
        
        # Регистрация получателя
        recipient_data = create_test_user_data()
        response = session.post(f"{api_base_url}/users", json=recipient_data, timeout=TIMEOUT)
        assert response.status_code == 201
        recipient_id = response.json()["id"]
        
        # Логин отправителя
        response = session.post(
            f"{api_base_url}/login",
            json={"login": sender_data["login"], "password": sender_data["password"]},
            timeout=TIMEOUT
        )
        assert response.status_code == 200
        token = response.json()["access_token"]
        headers = {"Authorization": f"Bearer {token}"}
        
        # Создание посылки
        parcel_data = create_test_parcel_data(sender_id, recipient_id)
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=headers,
            timeout=TIMEOUT
        )
        assert response.status_code == 201
        parcel_id = response.json()["id"]
        
        # Создание доставки
        delivery_data = create_test_delivery_data(parcel_id, recipient_id)
        response = session.post(
            f"{api_base_url}/deliveries",
            json=delivery_data,
            headers=headers,
            timeout=TIMEOUT
        )
        assert response.status_code == 201
        
        # Проверка доставок по получателю
        response = session.get(
            f"{api_base_url}/deliveries",
            params={"recipient_id": recipient_id},
            timeout=TIMEOUT
        )
        assert response.status_code == 200
        deliveries = response.json()
        assert any(d["parcel_id"] == parcel_id for d in deliveries)
        
        # Проверка доставок по отправителю
        response = session.get(
            f"{api_base_url}/deliveries",
            params={"sender_id": sender_id},
            timeout=TIMEOUT
        )
        assert response.status_code == 200
        deliveries = response.json()
        assert any(d["parcel_id"] == parcel_id for d in deliveries)
        
        # Проверка посылок по отправителю
        response = session.get(
            f"{api_base_url}/parcels",
            params={"sender_id": sender_id},
            timeout=TIMEOUT
        )
        assert response.status_code == 200
        parcels = response.json()
        assert any(p["id"] == parcel_id for p in parcels)
    
    def test_token_expiration_simulation(self, session: requests.Session, api_base_url: str):
        """Тест на просроченный токен (симуляция через невалидный токен)"""
        headers = {"Authorization": "Bearer expired_token_simulation"}
        parcel_data = create_test_parcel_data(sender_id=1, recipient_id=2)
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=headers,
            timeout=TIMEOUT
        )
        
        assert response.status_code in [401, 403], f"Expected 401 or 403, got {response.status_code}"
    
    def test_negative_weight_validation(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], second_user: Dict[str, Any]):
        """Создание посылки с отрицательным весом -> 422"""
        parcel_data = create_test_parcel_data(test_user_with_token["id"], second_user["id"])
        parcel_data["weight"] = -5.0
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        
        assert response.status_code == 422
        assert_error_response(response, "CONSTRAINTS_NOT_MATCHED")
    
    def test_zero_weight_validation(self, session: requests.Session, api_base_url: str, test_user_with_token: Dict[str, Any], second_user: Dict[str, Any]):
        """Создание посылки с нулевым весом -> 422"""
        parcel_data = create_test_parcel_data(test_user_with_token["id"], second_user["id"])
        parcel_data["weight"] = 0.0
        
        response = session.post(
            f"{api_base_url}/parcels",
            json=parcel_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        
        assert response.status_code == 422
        assert_error_response(response, "CONSTRAINTS_NOT_MATCHED")


# =============================================================================
# WAIT FOR API FIXTURE
# =============================================================================

@pytest.fixture(autouse=True, scope="session")
def wait_for_api(api_base_url: str):
    """Фикстура: ожидание готовности API перед запуском тестов"""
    max_retries = 30
    timeout = 5
    
    for attempt in range(max_retries):
        try:
            response = requests.get(f"{api_base_url}/docs", timeout=timeout)
            if response.status_code == 200:
                return
        except (requests.exceptions.ConnectionError, requests.exceptions.ReadTimeout):
            if attempt == max_retries - 1:
                pytest.fail(f"API не доступен по адресу {api_base_url} после {max_retries} попыток")
            time.sleep(2)


# =============================================================================
# PYTEST CONFIGURATION
# =============================================================================

def pytest_configure(config):
    """Дополнительная конфигурация pytest"""
    config.addinivalue_line(
        "markers", "slow: mark test as slow (for long-running tests)"
    )