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
        storage_(context.FindComponent<components::StorageComponent>()) {}

  std::string GetParcelsHandler::HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const
  {

    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);

    if (request.HasArg("user_id"))
    {
      try
      {
        int64_t user_id = std::stoll(request.GetArg("user_id"));
        auto parcels = storage_.GetUserParcels(user_id);
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{parcels}.ExtractValue());
      }
      catch (const std::exception &e)
      {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{"BAD_REQUEST", "Invalid user_id"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
      }
    }

    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    models::dto::ErrorResponse error{
        "BAD_REQUEST", "Missing query parameter (user_id)"};
    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{error}.ExtractValue());
  }

} // namespace handlers