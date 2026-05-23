#include "parcel.hpp"

namespace models::dto
{

  userver::formats::json::Value
  Serialize(const ParcelCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["sender_id"] = data.sender_id;
    builder["weight"] = data.weight;
    builder["dimensions"] = data.dimensions;
    builder["declared_value"] = data.declared_value;
    if (data.description)
    {
      builder["description"] = *data.description;
    }
    return builder.ExtractValue();
  }

  userver::formats::json::Value
  Serialize(const ParcelResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>)
  {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = data.id;
    builder["sender_id"] = data.sender_id;
    builder["weight"] = data.weight;
    builder["dimensions"] = data.dimensions;
    builder["declared_value"] = data.declared_value;
    if (data.description)
    {
      builder["description"] = *data.description;
    }
    builder["status"] = data.status;
    builder["created_at"] = data.created_at;
    return builder.ExtractValue();
  }

  ParcelCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelCreateRequest>)
  {
    ParcelCreateRequest result;
    result.sender_id = json["sender_id"].As<int64_t>();
    result.weight = json["weight"].As<double>();
    result.dimensions = json["dimensions"].As<std::string>();
    result.declared_value = json["declared_value"].As<double>();
    if (json.HasMember("description"))
    {
      result.description = json["description"].As<std::string>();
    }
    return result;
  }

  ParcelResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelResponse>)
  {
    ParcelResponse result;
    result.id = json["id"].As<int64_t>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.weight = json["weight"].As<double>();
    result.dimensions = json["dimensions"].As<std::string>();
    result.declared_value = json["declared_value"].As<double>();
    if (json.HasMember("description"))
    {
      result.description = json["description"].As<std::string>();
    }
    result.status = json["status"].As<std::string>();
    result.created_at = json["created_at"].As<std::string>();
    return result;
  }

} // namespace models::dto