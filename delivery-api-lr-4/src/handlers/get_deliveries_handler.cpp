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
          storage_(context.FindComponent<components::MongoStorageComponent>()) {}

    std::string GetDeliveriesHandler::HandleRequestThrow(
        const userver::server::http::HttpRequest &request,
        userver::server::request::RequestContext &) const
    {

        request.GetHttpResponse().SetContentType(
            userver::http::content_type::kApplicationJson);

        // Поиск по трек-номеру (не требует from/to)
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
            std::vector<models::dto::DeliveryResponse> deliveries;
            deliveries.push_back(*delivery);
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{deliveries}.ExtractValue());
        }

        // Поиск по отправителю (требует from/to)
        if (request.HasArg("sender_id"))
        {
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

        // Поиск по получателю (требует from/to)
        if (request.HasArg("receiver_id"))
        {
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

        // Поиск по статусу (требует from/to)
        if (request.HasArg("status"))
        {
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

            auto deliveries = storage_.GetDeliveriesByStatus(request.GetArg("status"), to, from);
            return userver::formats::json::ToString(
                userver::formats::json::ValueBuilder{deliveries}.ExtractValue());
        }

        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        models::dto::ErrorResponse error{
            "BAD_REQUEST",
            "Missing query parameter (tracking_number, sender_id, receiver_id, or status)"};
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{error}.ExtractValue());
    }

} // namespace handlers
