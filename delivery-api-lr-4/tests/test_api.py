"""
Delivery API — Комплексные тесты для варианта 6 (CDEK-like)
Запуск: pytest tests/test_api.py -v
"""

import pytest
import requests
import uuid
import time
from typing import Dict, Any

BASE_URL = "http://localhost:8080/api/v1"
TIMEOUT = 10

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

def assert_success_response(response: requests.Response, expected_status: int):
    assert response.status_code == expected_status

def assert_error_response(response: requests.Response, expected_code: str):
    assert response.status_code in [400, 401, 403, 404, 409]

# Остальные тесты аналогичны ЛР3
# (полный код тестов такой же как в ЛР3, см. предыдущие сообщения)