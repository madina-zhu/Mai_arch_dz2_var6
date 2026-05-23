"""
Delivery API — Комплексные тесты для варианта 6 (CDEK-like)
Покрывает все успешные сценарии и ошибки согласно OpenAPI спецификации.

Запуск:
    pytest tests/test_api.py -v
"""

import pytest
import requests
import uuid
import time
from typing import Dict, Any

BASE_URL = "http://localhost:8080/api/v1"
TIMEOUT = 10

BASE_TEST_USER = {
    "login": "test_user",
    "password": "password123",
    "first_name": "Test",
    "last_name": "User",
    "email": "test@example.com"
}

BASE_TEST_PARCEL = {
    "weight": 2.5,
    "dimensions": "30x20x15",
    "declared_value": 5000.00,
    "description": "Books and documents"
}

BASE_TEST_DELIVERY = {
    "from_address": "Moscow, Tverskaya 15",
    "to_address": "Saint Petersburg, Nevsky 25"
}


def generate_unique_string(prefix: str = "") -> str:
    return f"{prefix}_{uuid.uuid4().hex[:8]}"


def create_test_user_data() -> Dict[str, Any]:
    unique_id = uuid.uuid4().hex[:8]
    return {
        "login": f"user_{unique_id}",
        "password": "password123",
        "first_name": "Test",
        "last_name": "User",
        "email": f"test_{unique_id}@example.com"
    }


def create_test_parcel_data(sender_id: int) -> Dict[str, Any]:
    data = BASE_TEST_PARCEL.copy()
    data["sender_id"] = sender_id
    return data


def create_test_delivery_data(parcel_id: int, receiver_id: int) -> Dict[str, Any]:
    data = BASE_TEST_DELIVERY.copy()
    data["parcel_id"] = parcel_id
    data["receiver_id"] = receiver_id
    return data


def assert_success_response(response: requests.Response, expected_status: int):
    assert response.status_code == expected_status, \
        f"Expected {expected_status}, got {response.status_code}. Body: {response.text}"


def assert_error_response(response: requests.Response, expected_code: str):
    assert response.status_code in [400, 401, 403, 404, 409]
    if response.headers.get("Content-Type", "").startswith("application/json"):
        data = response.json()
        assert "code" in data
        assert data["code"] == expected_code


@pytest.fixture(scope="function")
def session() -> requests.Session:
    session = requests.Session()
    session.headers.update({"Content-Type": "application/json"})
    yield session
    session.close()


@pytest.fixture(scope="function")
def registered_user(session: requests.Session) -> Dict[str, Any]:
    user_data = create_test_user_data()
    response = session.post(f"{BASE_URL}/users", json=user_data, timeout=TIMEOUT)
    assert response.status_code == 201
    user_id = response.json()["id"]
    return {
        "id": user_id,
        "login": user_data["login"],
        "password": user_data["password"],
        "data": user_data
    }


@pytest.fixture(scope="function")
def auth_token(session: requests.Session, registered_user: Dict[str, Any]) -> str:
    response = session.post(
        f"{BASE_URL}/login",
        json={"login": registered_user["login"], "password": registered_user["password"]},
        timeout=TIMEOUT
    )
    assert response.status_code == 200
    return response.json()["access_token"]


@pytest.fixture(scope="function")
def auth_headers(auth_token: str) -> dict:
    return {"Authorization": f"Bearer {auth_token}"}


@pytest.fixture(scope="function")
def test_user_with_token(session: requests.Session) -> Dict[str, Any]:
    user_data = create_test_user_data()
    response = session.post(f"{BASE_URL}/users", json=user_data, timeout=TIMEOUT)
    assert response.status_code == 201
    user_id = response.json()["id"]
    response = session.post(
        f"{BASE_URL}/login",
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
    test_user_with_token: Dict[str, Any]
) -> dict:
    parcel_data = create_test_parcel_data(test_user_with_token["id"])
    response = session.post(
        f"{BASE_URL}/parcels",
        json=parcel_data,
        headers=test_user_with_token["headers"],
        timeout=TIMEOUT
    )
    assert response.status_code == 201
    return response.json()


@pytest.fixture(scope="function")
def second_user(session: requests.Session) -> Dict[str, Any]:
    user_data = create_test_user_data()
    response = session.post(f"{BASE_URL}/users", json=user_data, timeout=TIMEOUT)
    assert response.status_code == 201
    return response.json()


class TestAuthEndpoints:
    def test_login_success(self, session: requests.Session, registered_user: Dict[str, Any]):
        response = session.post(
            f"{BASE_URL}/login",
            json={"login": registered_user["login"], "password": registered_user["password"]},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert "access_token" in data
        assert data["token_type"] == "Bearer"

    def test_login_wrong_password(self, session: requests.Session, registered_user: Dict[str, Any]):
        response = session.post(
            f"{BASE_URL}/login",
            json={"login": registered_user["login"], "password": "wrong_password"},
            timeout=TIMEOUT
        )
        assert response.status_code == 401

    def test_login_nonexistent_user(self, session: requests.Session):
        response = session.post(
            f"{BASE_URL}/login",
            json={"login": "nonexistent", "password": "any"},
            timeout=TIMEOUT
        )
        assert response.status_code == 401


class TestUserEndpoints:
    def test_create_user_success(self, session: requests.Session):
        user_data = create_test_user_data()
        response = session.post(f"{BASE_URL}/users", json=user_data, timeout=TIMEOUT)
        assert_success_response(response, 201)
        data = response.json()
        assert data["login"] == user_data["login"]
        assert "id" in data

    def test_create_user_duplicate_login(self, session: requests.Session, registered_user: Dict[str, Any]):
        user_data = create_test_user_data()
        user_data["login"] = registered_user["login"]
        response = session.post(f"{BASE_URL}/users", json=user_data, timeout=TIMEOUT)
        assert response.status_code == 409

    def test_search_users_by_login(self, session: requests.Session, registered_user: Dict[str, Any]):
        response = session.get(
            f"{BASE_URL}/users",
            params={"login": registered_user["login"]},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1

    def test_search_users_by_name_mask(self, session: requests.Session, registered_user: Dict[str, Any]):
        response = session.get(
            f"{BASE_URL}/users",
            params={"name_mask": "Test*"},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)

    def test_search_users_no_params(self, session: requests.Session):
        response = session.get(f"{BASE_URL}/users", timeout=TIMEOUT)
        assert response.status_code == 400


class TestParcelEndpoints:
    def test_create_parcel_with_auth(self, session: requests.Session, test_user_with_token: Dict[str, Any]):
        parcel_data = create_test_parcel_data(test_user_with_token["id"])
        response = session.post(
            f"{BASE_URL}/parcels",
            json=parcel_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert_success_response(response, 201)
        data = response.json()
        assert data["weight"] == parcel_data["weight"]
        assert data["sender_id"] == test_user_with_token["id"]
        assert "id" in data

    def test_create_parcel_without_auth(self, session: requests.Session):
        parcel_data = create_test_parcel_data(1)
        response = session.post(f"{BASE_URL}/parcels", json=parcel_data, timeout=TIMEOUT)
        assert response.status_code == 401

    def test_create_parcel_invalid_sender(self, session: requests.Session, auth_headers: dict):
        parcel_data = create_test_parcel_data(99999)
        response = session.post(
            f"{BASE_URL}/parcels",
            json=parcel_data,
            headers=auth_headers,
            timeout=TIMEOUT
        )
        assert response.status_code == 404

    def test_get_user_parcels(self, session: requests.Session, test_user_with_token: Dict[str, Any], created_parcel: dict):
        response = session.get(
            f"{BASE_URL}/parcels",
            params={"user_id": test_user_with_token["id"]},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1
        assert data[0]["id"] == created_parcel["id"]


class TestDeliveryEndpoints:
    def test_create_delivery_with_auth(
        self,
        session: requests.Session,
        test_user_with_token: Dict[str, Any],
        created_parcel: dict,
        second_user: Dict[str, Any]
    ):
        delivery_data = create_test_delivery_data(created_parcel["id"], second_user["id"])
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert_success_response(response, 201)
        data = response.json()
        assert data["parcel_id"] == created_parcel["id"]
        assert data["receiver_id"] == second_user["id"]
        assert "tracking_number" in data
        assert data["status"] == "pending"

    def test_create_delivery_without_auth(self, session: requests.Session, created_parcel: dict):
        delivery_data = create_test_delivery_data(created_parcel["id"], 2)
        response = session.post(f"{BASE_URL}/deliveries", json=delivery_data, timeout=TIMEOUT)
        assert response.status_code == 401

    def test_create_delivery_nonexistent_parcel(self, session: requests.Session, test_user_with_token: Dict[str, Any], second_user: Dict[str, Any]):
        delivery_data = create_test_delivery_data(99999, second_user["id"])
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert response.status_code == 404

    def test_get_delivery_by_tracking_number(
        self,
        session: requests.Session,
        test_user_with_token: Dict[str, Any],
        created_parcel: dict,
        second_user: Dict[str, Any]
    ):
        # Создаем доставку
        delivery_data = create_test_delivery_data(created_parcel["id"], second_user["id"])
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert response.status_code == 201
        tracking_number = response.json()["tracking_number"]

        # Ищем по трек-номеру
        response = session.get(
            f"{BASE_URL}/deliveries",
            params={"tracking_number": tracking_number},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) == 1
        assert data[0]["tracking_number"] == tracking_number

    def test_get_deliveries_by_sender(
        self,
        session: requests.Session,
        test_user_with_token: Dict[str, Any],
        created_parcel: dict,
        second_user: Dict[str, Any]
    ):
        delivery_data = create_test_delivery_data(created_parcel["id"], second_user["id"])
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert response.status_code == 201

        response = session.get(
            f"{BASE_URL}/deliveries",
            params={"sender_id": test_user_with_token["id"]},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1

    def test_get_deliveries_by_receiver(
        self,
        session: requests.Session,
        test_user_with_token: Dict[str, Any],
        created_parcel: dict,
        second_user: Dict[str, Any]
    ):
        delivery_data = create_test_delivery_data(created_parcel["id"], second_user["id"])
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=test_user_with_token["headers"],
            timeout=TIMEOUT
        )
        assert response.status_code == 201

        response = session.get(
            f"{BASE_URL}/deliveries",
            params={"receiver_id": second_user["id"]},
            timeout=TIMEOUT
        )
        assert_success_response(response, 200)
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 1


class TestDocsEndpoints:
    def test_get_openapi_spec(self, session: requests.Session):
        response = session.get(f"{BASE_URL}/docs/openapi.yaml", timeout=TIMEOUT)
        assert response.status_code == 200
        assert "openapi:" in response.text

    def test_get_swagger_ui(self, session: requests.Session):
        response = session.get(f"{BASE_URL}/docs", timeout=TIMEOUT)
        assert response.status_code == 200
        assert "text/html" in response.headers["Content-Type"]


class TestEdgeCases:
    def test_full_workflow(self, session: requests.Session):
        # 1. Регистрация отправителя
        sender_data = create_test_user_data()
        response = session.post(f"{BASE_URL}/users", json=sender_data, timeout=TIMEOUT)
        assert response.status_code == 201
        sender_id = response.json()["id"]

        # 2. Логин отправителя
        response = session.post(
            f"{BASE_URL}/login",
            json={"login": sender_data["login"], "password": sender_data["password"]},
            timeout=TIMEOUT
        )
        assert response.status_code == 200
        token = response.json()["access_token"]
        headers = {"Authorization": f"Bearer {token}"}

        # 3. Создание посылки
        parcel_data = create_test_parcel_data(sender_id)
        response = session.post(
            f"{BASE_URL}/parcels",
            json=parcel_data,
            headers=headers,
            timeout=TIMEOUT
        )
        assert response.status_code == 201
        parcel_id = response.json()["id"]

        # 4. Регистрация получателя
        receiver_data = create_test_user_data()
        response = session.post(f"{BASE_URL}/users", json=receiver_data, timeout=TIMEOUT)
        assert response.status_code == 201
        receiver_id = response.json()["id"]

        # 5. Создание доставки
        delivery_data = create_test_delivery_data(parcel_id, receiver_id)
        response = session.post(
            f"{BASE_URL}/deliveries",
            json=delivery_data,
            headers=headers,
            timeout=TIMEOUT
        )
        assert response.status_code == 201
        tracking_number = response.json()["tracking_number"]

        # 6. Проверка по трек-номеру
        response = session.get(
            f"{BASE_URL}/deliveries",
            params={"tracking_number": tracking_number},
            timeout=TIMEOUT
        )
        assert response.status_code == 200

    def test_token_expiration(self, session: requests.Session):
        headers = {"Authorization": "Bearer invalid_token"}
        parcel_data = create_test_parcel_data(1)
        response = session.post(
            f"{BASE_URL}/parcels",
            json=parcel_data,
            headers=headers,
            timeout=TIMEOUT
        )
        assert response.status_code in [401, 403]


@pytest.fixture(autouse=True, scope="session")
def wait_for_api():
    max_retries = 30
    for attempt in range(max_retries):
        try:
            response = requests.get(f"{BASE_URL}/docs", timeout=5)
            if response.status_code == 200:
                return
        except (requests.exceptions.ConnectionError, requests.exceptions.ReadTimeout):
            if attempt == max_retries - 1:
                pytest.fail(f"API not available at {BASE_URL}")
            time.sleep(2)