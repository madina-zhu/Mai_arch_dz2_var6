#include "login_handler.hpp"
#include "../models/auth.hpp"
#include "../models/error.hpp"

#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers
{

  LoginHandler::LoginHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
      : HttpHandlerBase(config, context),
        auth_(context.FindComponent<components::AuthComponent>()) {}

  std::string LoginHandler::HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const
  {

    request.GetHttpResponse().SetContentType(
        userver::http::content_type::kApplicationJson);

    userver::formats::json::Value json;
    models::dto::LoginRequest dto;
    try
    {
      const auto &body = request.RequestBody();
      json = userver::formats::json::FromString(body);
      dto = json.As<models::dto::LoginRequest>();
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

    // Для MongoDB версии — упрощённая авторизация
    // Любой логин считается валидным, генерируем user_id
    static std::atomic<int64_t> next_id{1};
    int64_t user_id = next_id.fetch_add(1);

    std::string token = auth_.GenerateToken(user_id, dto.login);

    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    models::dto::AuthResponse response{token, "Bearer", user_id, dto.login};

    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{response}.ExtractValue());
  }

} // namespace handlers