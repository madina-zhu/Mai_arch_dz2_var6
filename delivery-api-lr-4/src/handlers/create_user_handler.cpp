#include "create_user_handler.hpp"
#include "../models/dto.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace handlers
{

  CreateUserHandler::CreateUserHandler(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : HttpHandlerBase(config, context),
        auth_(context.FindComponent<components::AuthComponent>()) {}

  std::string CreateUserHandler::HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const
  {

    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);

    userver::formats::json::Value json;
    models::dto::UserCreateRequest dto;
    try
    {
      const auto &body = request.RequestBody();
      json = userver::formats::json::FromString(body);
      dto = json.As<models::dto::UserCreateRequest>();
    }
    catch (const std::exception &e)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      models::dto::ErrorResponse error{"VALIDATION_ERROR",
                                       "Invalid request body"};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    if (dto.login.empty() || dto.password.empty())
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      models::dto::ErrorResponse error{"VALIDATION_ERROR",
                                       "Login and password required"};
      return userver::formats::json::ToString(
          userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

    // Для MongoDB версии используем простую генерацию ID
    static std::atomic<int64_t> next_id{1};
    int64_t user_id = next_id.fetch_add(1);

    // Формируем дату
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    std::string created_at = ss.str();

    request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);

    models::dto::UserResponse response;
    response.id = user_id;
    response.login = dto.login;
    response.first_name = dto.first_name;
    response.last_name = dto.last_name;
    response.email = dto.email;
    response.created_at = created_at;

    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{response}.ExtractValue());
  }

} // namespace handlers