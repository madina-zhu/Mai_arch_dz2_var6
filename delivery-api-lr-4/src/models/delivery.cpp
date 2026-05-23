#include "delivery.hpp"

namespace models::dto
{

  userver::formats::json::Value
  Serialize(const DeliveryEvent &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["status"] = data.status;
    builder["timestamp"] = data.timestamp;
    builder["location"] = data.location;
    if (data.comment)
    {
      builder["comment"] = *data.comment;
    }
    return builder.ExtractValue();
  }

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
    userver::formats::json::ValueBuilder events_builder;
    for (const auto &event : data.events)
    {
      events_builder.PushBack(event);
    }
    builder["events"] = events_builder.ExtractValue();
    return builder.ExtractValue();
  }

  DeliveryEvent
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryEvent>)
  {
    DeliveryEvent result;
    result.status = json["status"].As<std::string>();
    result.timestamp = json["timestamp"].As<std::chrono::system_clock::time_point>();
    result.location = json["location"].As<std::string>();
    if (json.HasMember("comment"))
    {
      result.comment = json["comment"].As<std::string>();
    }
    return result;
  }

  DeliveryCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<DeliveryCreateRequest>)
  {
    DeliveryCreateRequest result;
    result.parcel_id = json["parcel_id"].As<std::string>();
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
    result.id = json["id"].As<std::string>();
    result.parcel_id = json["parcel_id"].As<std::string>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.receiver_id = json["receiver_id"].As<int64_t>();
    result.tracking_number = json["tracking_number"].As<std::string>();
    result.status = json["status"].As<std::string>();
    result.from_address = json["from_address"].As<std::string>();
    result.to_address = json["to_address"].As<std::string>();
    result.created_at = json["created_at"].As<std::chrono::system_clock::time_point>();
    if (json.HasMember("delivered_at") && !json["delivered_at"].IsNull())
    {
      result.delivered_at = json["delivered_at"].As<std::chrono::system_clock::time_point>();
    }
    if (json.HasMember("events"))
    {
      for (const auto &event_json : json["events"])
      {
        result.events.push_back(event_json.As<DeliveryEvent>());
      }
    }
    return result;
  }

} // namespace models::dto