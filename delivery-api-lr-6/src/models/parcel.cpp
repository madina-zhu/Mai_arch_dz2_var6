#include "parcel.hpp"
#include <chrono>
#include <string>

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
    if (data.features)
    {
      userver::formats::json::ValueBuilder features_builder;
      for (const auto &feature : *data.features)
      {
        features_builder.PushBack(feature);
      }
      builder["features"] = features_builder.ExtractValue();
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
    if (data.features)
    {
      userver::formats::json::ValueBuilder features_builder;
      for (const auto &feature : *data.features)
      {
        features_builder.PushBack(feature);
      }
      builder["features"] = features_builder.ExtractValue();
    }
    return builder.ExtractValue();
  }

  ParcelCreateRequest Parse(const userver::formats::json::Value &json,
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
    if (json.HasMember("features"))
    {
      std::vector<std::string> features;
      for (const auto &feature : json["features"])
      {
        features.push_back(feature.As<std::string>());
      }
      result.features = features;
    }
    return result;
  }

  ParcelResponse Parse(const userver::formats::json::Value &json,
                       userver::formats::parse::To<ParcelResponse>)
  {
    ParcelResponse result;
    result.id = json["id"].As<std::string>();
    result.sender_id = json["sender_id"].As<int64_t>();
    result.weight = json["weight"].As<double>();
    result.dimensions = json["dimensions"].As<std::string>();
    result.declared_value = json["declared_value"].As<double>();
    if (json.HasMember("description"))
    {
      result.description = json["description"].As<std::string>();
    }
    result.status = json["status"].As<std::string>();
    result.created_at = json["created_at"].As<std::chrono::system_clock::time_point>();
    if (json.HasMember("features"))
    {
      std::vector<std::string> features;
      for (const auto &feature : json["features"])
      {
        features.push_back(feature.As<std::string>());
      }
      result.features = features;
    }
    return result;
  }

} // namespace models::dto
