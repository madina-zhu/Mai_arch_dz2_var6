#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/to.hpp>
#include <userver/formats/serialize/to.hpp>
#include <userver/storages/postgres/io/chrono.hpp>

namespace models::dto
{

  struct TrackingEvent
  {
    std::string status;
    std::chrono::system_clock::time_point timestamp;
    std::string location;
    std::optional<std::string> reason;
  };

  struct DeliveryCreateRequest
  {
    std::string parcel_id;
    int64_t recipient_id;
    int64_t courier_id;
    std::chrono::system_clock::time_point pickup_time;
    std::chrono::system_clock::time_point estimated_delivery;
  };

  struct DeliveryResponse
  {
    std::string id;
    std::string parcel_id;
    int64_t recipient_id;
    int64_t courier_id;
    std::chrono::system_clock::time_point pickup_time;
    std::chrono::system_clock::time_point estimated_delivery;
    std::optional<std::chrono::system_clock::time_point> actual_delivery;
    std::string status;
    std::vector<TrackingEvent> tracking_history;
    std::chrono::system_clock::time_point created_at;
  };

  userver::formats::json::Value
  Serialize(const TrackingEvent &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const DeliveryCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const DeliveryResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  TrackingEvent
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<TrackingEvent>);

  DeliveryCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryCreateRequest>);

  DeliveryResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryResponse>);

} // namespace models::dto