#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <userver/formats/json.hpp>

namespace models::dto
{

  struct DeliveryEvent
  {
    std::string status;
    std::chrono::system_clock::time_point timestamp;
    std::string location;
    std::optional<std::string> comment;
  };

  struct DeliveryCreateRequest
  {
    std::string parcel_id; // UUID из MongoDB
    int64_t sender_id;
    int64_t receiver_id;
    std::string from_address;
    std::string to_address;
    std::optional<std::string> scheduled_pickup_time;
  };

  struct DeliveryResponse
  {
    std::string id;
    std::string parcel_id;
    int64_t sender_id;
    int64_t receiver_id;
    std::string tracking_number;
    std::string status; // pending, in_transit, delivered, cancelled
    std::string from_address;
    std::string to_address;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> delivered_at;
    std::vector<DeliveryEvent> events;
  };

  // Сериализация/парсинг JSON
  userver::formats::json::Value
  Serialize(const DeliveryEvent &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const DeliveryCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const DeliveryResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  DeliveryEvent
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryEvent>);

  DeliveryCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryCreateRequest>);

  DeliveryResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryResponse>);

} // namespace models::dto