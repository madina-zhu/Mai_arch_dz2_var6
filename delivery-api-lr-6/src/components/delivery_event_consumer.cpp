#include "delivery_event_consumer.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>

namespace components
{

  DeliveryEventConsumer::DeliveryEventConsumer(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : ConsumerComponentBase(config, context),
        read_db_(context.FindComponent<ReadDatabaseComponent>("read-database-component"))
  {

    handlers_ = {
        {"user.registered", [this](const auto &p)
         { HandleUserRegistered(p); }},
        {"parcel.created", [this](const auto &p)
         { HandleParcelCreated(p); }},
        {"delivery.created", [this](const auto &p)
         { HandleDeliveryCreated(p); }},
        {"delivery.status_changed", [this](const auto &p)
         { HandleDeliveryStatusChanged(p); }}};
  }

  void DeliveryEventConsumer::Process(std::string message)
  {
    try
    {
      auto json = userver::formats::json::FromString(message);

      std::string event_type = "unknown";
      if (json.HasMember("event_type"))
      {
        event_type = json["event_type"].As<std::string>();
      }

      LOG_INFO() << "Consumed event: " << event_type;

      auto it = handlers_.find(event_type);
      if (it != handlers_.end())
      {
        it->second(json["payload"]);
      }
      else
      {
        LOG_WARNING() << "No handler for event type: " << event_type;
      }
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Error processing message: " << e.what();
    }
  }

  void DeliveryEventConsumer::HandleUserRegistered(const userver::formats::json::Value &payload)
  {
    LOG_INFO() << "User registered event: " << payload["login"].As<std::string>();
  }

  void DeliveryEventConsumer::HandleParcelCreated(const userver::formats::json::Value &payload)
  {
    LOG_INFO() << "Parcel created event: id=" << payload["id"].As<std::string>()
               << ", sender=" << payload["sender_id"].As<int64_t>();
  }

  void DeliveryEventConsumer::HandleDeliveryCreated(const userver::formats::json::Value &payload)
  {
    LOG_INFO() << "Delivery created event: id=" << payload["id"].As<std::string>()
               << ", tracking=" << payload["tracking_number"].As<std::string>();
  }

  void DeliveryEventConsumer::HandleDeliveryStatusChanged(const userver::formats::json::Value &payload)
  {
    LOG_INFO() << "Delivery status changed: id=" << payload["delivery_id"].As<std::string>()
               << ", " << payload["old_status"].As<std::string>()
               << " -> " << payload["new_status"].As<std::string>();
  }

} // namespace components