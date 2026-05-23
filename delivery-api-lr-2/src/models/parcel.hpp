#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/to.hpp>
#include <userver/formats/serialize/to.hpp>

namespace models::dto
{

  struct ParcelCreateRequest
  {
    int64_t sender_id;
    double weight;
    std::string dimensions;
    double declared_value;
    std::optional<std::string> description;
  };

  struct ParcelResponse
  {
    int64_t id;
    int64_t sender_id;
    double weight;
    std::string dimensions;
    double declared_value;
    std::optional<std::string> description;
    std::string status;
    std::string created_at;
  };

  userver::formats::json::Value
  Serialize(const ParcelCreateRequest &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  userver::formats::json::Value
  Serialize(const ParcelResponse &data,
            userver::formats::serialize::To<userver::formats::json::Value>);

  ParcelCreateRequest
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelCreateRequest>);

  ParcelResponse
  Parse(const userver::formats::json::Value &json,
        userver::formats::parse::To<ParcelResponse>);

} // namespace models::dto