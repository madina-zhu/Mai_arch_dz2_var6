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

  struct ParcelCreateRequest
  {
    int64_t sender_id;
    int64_t recipient_id;
    std::string type;
    std::string title;
    std::string description;
    std::string from_city;
    std::string to_city;
    userver::formats::json::Value address;
    double weight;
    double price;
    userver::formats::json::Value details;
    userver::formats::json::Value features;
    std::string status;
  };

  struct ParcelResponse
  {
    std::string id;
    int64_t sender_id;
    int64_t recipient_id;
    std::string type;
    std::string title;
    std::string description;
    std::string from_city;
    std::string to_city;
    userver::formats::json::Value address;
    double weight;
    double price;
    userver::formats::json::Value details;
    userver::formats::json::Value features;
    std::string status;
    userver::storages::postgres::TimePointTz created_at;
  };

  struct ParcelUpdateRequest
  {
    std::optional<std::string> status;
    std::optional<double> weight;
  };

  userver::formats::json::Value
  Serialize(const ParcelCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const ParcelResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const ParcelUpdateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  ParcelCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelCreateRequest>);

  ParcelResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelResponse>);

  ParcelUpdateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelUpdateRequest>);

} // namespace models::dto