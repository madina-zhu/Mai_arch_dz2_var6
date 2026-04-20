#include "create_delivery_handler.hpp"
#include "../models/dto.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers
{

  CreateDeliveryHandler::CreateDeliveryHandler(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : HttpHandlerBase(config, context),
        storage_(context.FindComponent<components::MongoStorageComponent>()) {}

  std::string CreateDeliveryHandler::HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const
  {

    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);

    userver::formats::json::Value json;
    models::dto::DeliveryCreateRequest dto;
    try
    {
      const auto &body = request.RequestBody();
      json = userver::formats::json::FromString(body);
      dto = json.As<models::dto::DeliveryCreateRequest>();
    }
    catch (const std::exception &e)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      models::dto::ErrorResponse error{"VALIDATION_ERROR",
                                       "Invalid request body"};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    std::string delivery_id = storage_.CreateDelivery(dto);

    if (delivery_id == components::MongoStorageComponent::dataViolation)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
      models::dto::ErrorResponse error{"NOT_FOUND", "Parcel not found"};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    if (delivery_id == components::MongoStorageComponent::constraintViolation)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kUnprocessableEntity);
      models::dto::ErrorResponse error{"CONSTRAINTS_NOT_MATCHED",
                                       "Constraints are violated"};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    auto delivery = storage_.GetDeliveryById(delivery_id);
    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{*delivery}.ExtractValue());
  }

} // namespace handlers