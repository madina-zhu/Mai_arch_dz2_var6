#include "delivery.hpp"

#include <chrono>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace models::dto
{

  // ===================== SERIALIZE =====================

  userver::formats::json::Value
  Serialize(const TrackingEvent &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["status"] = data.status;
    builder["timestamp"] = data.timestamp;
    builder["location"] = data.location;
    if (data.reason.has_value())
    {
      builder["reason"] = *data.reason;
    }
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const DeliveryCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["parcel_id"] = data.parcel_id;
    builder["recipient_id"] = data.recipient_id;
    builder["courier_id"] = data.courier_id;
    builder["pickup_time"] = data.pickup_time;
    builder["estimated_delivery"] = data.estimated_delivery;
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const DeliveryResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = data.id;
    builder["parcel_id"] = data.parcel_id;
    builder["recipient_id"] = data.recipient_id;
    builder["courier_id"] = data.courier_id;
    builder["pickup_time"] = data.pickup_time;
    builder["estimated_delivery"] = data.estimated_delivery;
    if (data.actual_delivery.has_value())
    {
      builder["actual_delivery"] = *data.actual_delivery;
    }
    else
    {
      builder["actual_delivery"] = nullptr;
    }
    builder["status"] = data.status;
    for (const auto &event : data.tracking_history)
    {
      builder["tracking_history"].PushBack(event);
    }
    builder["created_at"] = data.created_at;
    return builder.ExtractValue();
  }

  // ===================== PARSE =====================

  TrackingEvent Parse(const userver::formats::json::Value &json,
                      userver::formats::parse::To<TrackingEvent>)
  {
    TrackingEvent result;
    result.status = json["status"].As<std::string>();
    result.timestamp = json["timestamp"].As<std::chrono::system_clock::time_point>();
    result.location = json["location"].As<std::string>();
    if (json.HasMember("reason"))
    {
      result.reason = json["reason"].As<std::string>();
    }
    return result;
  }

  DeliveryCreateRequest Parse(const userver::formats::json::Value &json,
                              userver::formats::parse::To<DeliveryCreateRequest>)
  {
    DeliveryCreateRequest result;
    result.parcel_id = json["parcel_id"].As<std::string>();
    result.recipient_id = json["recipient_id"].As<int64_t>();
    result.courier_id = json["courier_id"].As<int64_t>();
    result.pickup_time = json["pickup_time"].As<std::chrono::system_clock::time_point>();
    result.estimated_delivery = json["estimated_delivery"].As<std::chrono::system_clock::time_point>();
    return result;
  }

  DeliveryResponse Parse(const userver::formats::json::Value &json,
                         userver::formats::parse::To<DeliveryResponse>)
  {
    DeliveryResponse result;
    result.id = json["id"].As<std::string>();
    result.parcel_id = json["parcel_id"].As<std::string>();
    result.recipient_id = json["recipient_id"].As<int64_t>();
    result.courier_id = json["courier_id"].As<int64_t>();
    result.pickup_time = json["pickup_time"].As<std::chrono::system_clock::time_point>();
    result.estimated_delivery = json["estimated_delivery"].As<std::chrono::system_clock::time_point>();
    if (!json["actual_delivery"].IsNull())
    {
      result.actual_delivery = json["actual_delivery"].As<std::chrono::system_clock::time_point>();
    }
    result.status = json["status"].As<std::string>();
    for (const auto &event_json : json["tracking_history"])
    {
      result.tracking_history.push_back(event_json.As<TrackingEvent>());
    }
    result.created_at = json["created_at"].As<std::chrono::system_clock::time_point>();
    return result;
  }

} // namespace models::dto