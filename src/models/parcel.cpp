#include "parcel.hpp"
#include <chrono>
#include <string>
#include <userver/formats/json/parser/parser.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

namespace models::dto
{

  userver::formats::json::Value
  Serialize(const ParcelCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["sender_id"] = data.sender_id;
    builder["recipient_id"] = data.recipient_id;
    builder["type"] = data.type;
    builder["title"] = data.title;
    builder["description"] = data.description;
    builder["from_city"] = data.from_city;
    builder["to_city"] = data.to_city;
    builder["address"] = data.address;
    builder["weight"] = data.weight;
    builder["price"] = data.price;
    builder["details"] = data.details;
    builder["features"] = data.features;
    builder["status"] = data.status;
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const ParcelResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = data.id;
    builder["sender_id"] = data.sender_id;
    builder["recipient_id"] = data.recipient_id;
    builder["type"] = data.type;
    builder["title"] = data.title;
    builder["description"] = data.description;
    builder["from_city"] = data.from_city;
    builder["to_city"] = data.to_city;
    builder["address"] = data.address;
    builder["weight"] = data.weight;
    builder["price"] = data.price;
    builder["details"] = data.details;
    builder["features"] = data.features;
    builder["status"] = data.status;
    builder["created_at"] = data.created_at;
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const ParcelUpdateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    if (data.status)
    {
      builder["status"] = *data.status;
    }
    if (data.weight)
    {
      builder["weight"] = *data.weight;
    }
    return builder.ExtractValue();
  }

  ParcelCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelCreateRequest>)
  {
    ParcelCreateRequest result;
    result.sender_id = json["sender_id"].As<int64_t>();
    result.recipient_id = json["recipient_id"].As<int64_t>();
    result.type = json["type"].As<std::string>();
    result.title = json["title"].As<std::string>();
    result.description = json["description"].As<std::string>();
    result.from_city = json["from_city"].As<std::string>();
    result.to_city = json["to_city"].As<std::string>();
    result.address = json["address"];
    result.weight = json["weight"].As<double>();
    result.price = json["price"].As<double>();
    result.details = json["details"];
    if (json.HasMember("features"))
    {
      result.features = json["features"];
    }
    result.status = json["status"].As<std::string>();
    return result;
  }

  ParcelResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelResponse>)
  {
    ParcelResponse result;
    result.id = json["id"].As<std::string>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.recipient_id = json["recipient_id"].As<int64_t>();
    result.type = json["type"].As<std::string>();
    result.title = json["title"].As<std::string>();
    result.description = json["description"].As<std::string>();
    result.from_city = json["from_city"].As<std::string>();
    result.to_city = json["to_city"].As<std::string>();
    result.address = json["address"];
    result.weight = json["weight"].As<double>();
    result.price = json["price"].As<double>();
    result.details = json["details"];
    if (json.HasMember("features"))
    {
      result.features = json["features"];
    }
    result.status = json["status"].As<std::string>();
    result.created_at = json["created_at"].As<userver::storages::postgres::TimePointTz>();
    return result;
  }

  ParcelUpdateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelUpdateRequest>)
  {
    ParcelUpdateRequest result;
    if (json.HasMember("status"))
    {
      result.status = json["status"].As<std::string>();
    }
    if (json.HasMember("weight"))
    {
      result.weight = json["weight"].As<double>();
    }
    return result;
  }

} // namespace models::dto