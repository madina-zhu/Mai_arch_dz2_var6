#include "get_parcels_handler.hpp"
#include "../models/dto.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers
{

  GetParcelsHandler::GetParcelsHandler(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : HttpHandlerBase(config, context),
        storage_(context.FindComponent<components::MongoStorageComponent>()) {}

  std::string GetParcelsHandler::HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const
  {

    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);

    int from, to;
    try
    {
      from = std::stoi(request.GetArg("from"));
      to = std::stoi(request.GetArg("to"));
      if (from <= 0 || to <= 0 || to < from)
      {
        throw std::invalid_argument("Invalid from/to bounds!");
      }
    }
    catch (const std::exception &e)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      models::dto::ErrorResponse error{"BAD_REQUEST", e.what()};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    // По городу отправления
    if (request.HasArg("from_city"))
    {
      auto parcels = storage_.GetParcelsByFromCity(request.GetArg("from_city"), to, from);
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{parcels}.ExtractValue());
    }

    // По диапазону веса
    if (request.HasArg("min_weight") && request.HasArg("max_weight"))
    {
      try
      {
        double min = std::stod(request.GetArg("min_weight"));
        double max = std::stod(request.GetArg("max_weight"));
        auto parcels = storage_.GetParcelsByWeightRange(min, max, to, from);
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{parcels}.ExtractValue());
      }
      catch (const std::exception &e)
      {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{"VALIDATION_ERROR",
                                         "Invalid weight format"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
      }
    }

    // По отправителю
    if (request.HasArg("sender_id"))
    {
      try
      {
        int64_t sender_id = std::stoll(request.GetArg("sender_id"));
        auto parcels = storage_.GetParcelsBySenderId(sender_id);
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{parcels}.ExtractValue());
      }
      catch (const std::exception &e)
      {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{"VALIDATION_ERROR",
                                         "Invalid sender_id format"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
      }
    }

    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    models::dto::ErrorResponse error{
        "BAD_REQUEST", "Missing query parameters (from_city, min_weight/max_weight, or sender_id)"};
    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{error}.ExtractValue());
  }

} // namespace handlers