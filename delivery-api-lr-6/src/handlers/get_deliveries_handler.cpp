#include "get_deliveries_handler.hpp"
#include "../models/dto.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/datetime.hpp>

namespace handlers
{

    GetDeliveriesHandler::GetDeliveriesHandler(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context)
        : HttpHandlerBase(config, context),
          storage_(context.FindComponent<components::MongoStorageComponent>()),
          limiter_(context.FindComponent<components::RateLimiterComponent>()) {}

    void GetDeliveriesHandler::SetRateLimiterHeaders(
        userver::server::http::HttpResponse &response,
        const std::string &ip) const
    {
        response.SetHeader(std::string("X-RateLimit-Limit"),
                           std::to_string(components::RateLimiterComponent::limit));
        response.SetHeader(std::string("X-RateLimit-Remaining"),
                           std::to_string(limiter_.GetRemaining(ip)));

        const auto now = userver::utils::datetime::Now();
        const auto reset_time =
            now + std::chrono::seconds(components::RateLimiterComponent::timeSec);

        response.SetHeader(std::string("X-RateLimit-Reset"),
                           std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                              reset_time.time_since_epoch())
                                              .count()));
        response.SetHeader(std::string("Retry-After"),
                           std::to_string(components::RateLimiterComponent::timeSec));
    }

    std::string GetDeliveriesHandler::HandleRequestThrow(
        const userver::server::http::HttpRequest &request,
        userver::server::request::RequestContext &) const
    {

        request.GetHttpResponse().SetContentType(
            userver::http::content_type::kApplicationJson);

        // Rate Limiting
        const auto &ip = request.GetRemoteAddress().PrimaryAddressString();
        bool isLimitExpired = !limiter_.CheckRateLimit(ip);
        SetRateLimiterHeaders(request.GetHttpResponse(), ip);

        if (isLimitExpired)
        {
            request.SetResponseStatus(
                userver::server::http::HttpStatus::kTooManyRequests);
            models::dto::ErrorResponse error{"TOO_MANY_REQUESTS",
                                             "Too many requests were made"};
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{error}.ExtractValue());
        }

        int from = 1, to = 10;
        try
        {
            if (request.HasArg("from"))
            {
                from = std::stoi(request.GetArg("from"));
            }
            if (request.HasArg("to"))
            {
                to = std::stoi(request.GetArg("to"));
            }
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

        // Поиск по трек-номеру
        if (request.HasArg("tracking_number"))
        {
            const auto tracking_number = request.GetArg("tracking_number");
            auto delivery = storage_.GetDeliveryByTrackingNumber(tracking_number);

            if (!delivery.has_value())
            {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                models::dto::ErrorResponse error{"NOT_FOUND", "Delivery not found"};
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{error}.ExtractValue());
            }
            std::vector<models::dto::DeliveryResponse> deliveries;
            deliveries.push_back(*delivery);
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{deliveries}.ExtractValue());
        }

        // Поиск по отправителю
        if (request.HasArg("sender_id"))
        {
            try
            {
                int64_t sender_id = std::stoll(request.GetArg("sender_id"));
                auto deliveries = storage_.GetDeliveriesBySender(sender_id, to, from);
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{deliveries}.ExtractValue());
            }
            catch (const std::exception &e)
            {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                models::dto::ErrorResponse error{"BAD_REQUEST", "Invalid sender_id"};
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{error}.ExtractValue());
            }
        }

        // Поиск по получателю
        if (request.HasArg("receiver_id"))
        {
            try
            {
                int64_t receiver_id = std::stoll(request.GetArg("receiver_id"));
                auto deliveries = storage_.GetDeliveriesByReceiver(receiver_id, to, from);
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{deliveries}.ExtractValue());
            }
            catch (const std::exception &e)
            {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                models::dto::ErrorResponse error{"BAD_REQUEST", "Invalid receiver_id"};
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{error}.ExtractValue());
            }
        }

        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{
            "BAD_REQUEST",
            "Missing query parameter (tracking_number, sender_id, or receiver_id)"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

} // namespace handlers