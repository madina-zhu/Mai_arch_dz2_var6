#include "../models/dto.hpp"
#include "get_users_handler.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp> // ← ЭТОТ ЗАГОЛОВОК НУЖЕН!
#include <userver/server/http/http_status.hpp>

namespace handlers
{

    GetUsersHandler::GetUsersHandler(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context)
        : HttpHandlerBase(config, context) {}

    std::string GetUsersHandler::HandleRequestThrow(
        const userver::server::http::HttpRequest &request,
        userver::server::request::RequestContext &) const
    {

        request.GetHttpResponse().SetContentType(
            userver::http::content_type::kApplicationJson);

        if (request.HasArg("login") || request.HasArg("name_mask"))
        {
            std::vector<models::dto::UserResponse> empty_result;
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{empty_result}.ExtractValue());
        }

        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{
            "BAD_REQUEST", "Missing query parameter (login or name_mask)"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

} // namespace handlers