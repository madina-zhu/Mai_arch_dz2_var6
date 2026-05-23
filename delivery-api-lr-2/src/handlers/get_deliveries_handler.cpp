#include "get_deliveries_handler.hpp"
#include "../models/dto.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers
{

    GetDeliveriesHandler::GetDeliveriesHandler(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context)
        : HttpHandlerBase(config, context),
          storage_(context.FindComponent<components::StorageComponent>()) {}

    std::string GetDeliveriesHandler::HandleRequestThrow(
        const userver::server::http::HttpRequest &request,
        userver::server::request::RequestContext &) const
    {

        request.GetHttpResponse().SetContentType(
            userver::http::content_type::kApplicationJson);

        // Поиск по трек-номеру
        if (request.HasArg("tracking_number"))
        {
            auto delivery = storage_.GetDeliveryByTrackingNumber(
                request.GetArg("tracking_number"));
            if (!delivery)
            {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                models::dto::ErrorResponse error{"NOT_FOUND", "Delivery not found"};
                return userver::formats::json::ToString(
                    userver::formats::json::ValueBuilder{error}.ExtractValue());
            }
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{std::vector{*delivery}}
                    .ExtractValue());
        }

        // Поиск по отправителю
        if (request.HasArg("sender_id"))
        {
            try
            {
                int64_t sender_id = std::stoll(request.GetArg("sender_id"));
                auto deliveries = storage_.GetDeliveriesBySender(sender_id);
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
                auto deliveries = storage_.GetDeliveriesByReceiver(receiver_id);
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