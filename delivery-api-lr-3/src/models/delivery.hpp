#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/to.hpp>
#include <userver/formats/serialize/to.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace models::dto
{

  struct DeliveryCreateRequest
  {
    int64_t parcel_id;
    int64_t sender_id;
    int64_t receiver_id;
    std::string from_address;
    std::string to_address;
    std::optional<std::string> scheduled_pickup_time;
  };

  struct DeliveryResponse
  {
    int64_t id;
    int64_t parcel_id;
    int64_t sender_id;
    int64_t receiver_id;
    std::string tracking_number;
    std::string status;
    std::string from_address;
    std::string to_address;
    userver::storages::postgres::TimePointTz created_at;
    std::optional<userver::storages::postgres::TimePointTz> delivered_at;
  };

  userver::formats::json::Value
  Serialize(const DeliveryCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const DeliveryResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  DeliveryCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryCreateRequest>);

  DeliveryResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryResponse>);

} // namespace models::dto