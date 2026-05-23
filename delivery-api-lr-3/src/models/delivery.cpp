#include "delivery.hpp"
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace models::dto
{

  userver::formats::json::Value
  Serialize(const DeliveryCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["parcel_id"] = data.parcel_id;
    builder["sender_id"] = data.sender_id;
    builder["receiver_id"] = data.receiver_id;
    builder["from_address"] = data.from_address;
    builder["to_address"] = data.to_address;
    if (data.scheduled_pickup_time)
    {
      builder["scheduled_pickup_time"] = *data.scheduled_pickup_time;
    }
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const DeliveryResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = data.id;
    builder["parcel_id"] = data.parcel_id;
    builder["sender_id"] = data.sender_id;
    builder["receiver_id"] = data.receiver_id;
    builder["tracking_number"] = data.tracking_number;
    builder["status"] = data.status;
    builder["from_address"] = data.from_address;
    builder["to_address"] = data.to_address;
    builder["created_at"] = data.created_at;
    if (data.delivered_at)
    {
      builder["delivered_at"] = *data.delivered_at;
    }
    return builder.ExtractValue();
  }

  DeliveryCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryCreateRequest>)
  {
    DeliveryCreateRequest result;
    result.parcel_id = json["parcel_id"].As<int64_t>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.receiver_id = json["receiver_id"].As<int64_t>();
    result.from_address = json["from_address"].As<std::string>();
    result.to_address = json["to_address"].As<std::string>();
    if (json.HasMember("scheduled_pickup_time"))
    {
      result.scheduled_pickup_time = json["scheduled_pickup_time"].As<std::string>();
    }
    return result;
  }

  DeliveryResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryResponse>)
  {
    DeliveryResponse result;
    result.id = json["id"].As<int64_t>();
    result.parcel_id = json["parcel_id"].As<int64_t>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.receiver_id = json["receiver_id"].As<int64_t>();
    result.tracking_number = json["tracking_number"].As<std::string>();
    result.status = json["status"].As<std::string>();
    result.from_address = json["from_address"].As<std::string>();
    result.to_address = json["to_address"].As<std::string>();
    result.created_at = json["created_at"].As<userver::storages::postgres::TimePointTz>();
    if (json.HasMember("delivered_at") && !json["delivered_at"].IsNull())
    {
      result.delivered_at = json["delivered_at"].As<userver::storages::postgres::TimePointTz>();
    }
    return result;
  }

} // namespace models::dto