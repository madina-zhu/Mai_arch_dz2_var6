#include "delivery_event_producer.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/inline.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>
#include <userver/urabbitmq/admin_channel.hpp>

namespace components
{

  DeliveryEventProducer::DeliveryEventProducer(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : LoggableComponentBase(config, context),
        publisher_{
            context.FindComponent<userver::components::RabbitMQ>("rabbit").GetClient()},
        exchange_{userver::urabbitmq::Exchange{"delivery_events"}}
  {
    try
    {
      auto admin = publisher_->GetAdminChannel(
          userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));

      auto queue = userver::urabbitmq::Queue("delivery_events_queue");

      admin.DeclareQueue(queue, userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));
      admin.DeclareExchange(exchange_, userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));
      admin.BindQueue(exchange_, queue, "#", userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));

      LOG_INFO() << "Queue 'delivery_events_queue' declared and bound to 'delivery_events' exchange";
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to declare and bind queue: " << e.what();
    }
  }

  void DeliveryEventProducer::PublishEvent(const std::string &event_type,
                                           const std::string &routing_key,
                                           const userver::formats::json::Value &payload)
  {
    auto event = userver::formats::json::MakeObject(
        "event_id", userver::utils::generators::GenerateUuid(),
        "event_type", event_type,
        "timestamp", userver::utils::datetime::Now(),
        "payload", payload);

    std::string body = userver::formats::json::ToString(event);

    try
    {
      publisher_->Publish(exchange_, routing_key, body,
                          userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));
      LOG_INFO() << "Event published: " << event_type << " [" << routing_key << "]";
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to publish event " << event_type << ": " << e.what();
    }
  }

  void DeliveryEventProducer::PublishUserRegistered(const models::dto::UserCreateRequest &user, int64_t user_id)
  {
    auto payload = userver::formats::json::MakeObject(
        "user_id", user_id,
        "login", user.login,
        "email", user.email,
        "first_name", user.first_name,
        "last_name", user.last_name);
    PublishEvent("user.registered", "user.registered", payload);
  }

  void DeliveryEventProducer::PublishParcelCreated(const std::string &id,
                                                   const models::dto::ParcelCreateRequest &parcel)
  {
    auto payload = userver::formats::json::MakeObject(
        "id", id,
        "sender_id", parcel.sender_id,
        "weight", parcel.weight,
        "dimensions", parcel.dimensions,
        "declared_value", parcel.declared_value,
        "description", parcel.description.value_or(""),
        "status", "created");
    PublishEvent("parcel.created", "parcel.created", payload);
  }

  void DeliveryEventProducer::PublishDeliveryCreated(const models::dto::DeliveryResponse &delivery)
  {
    auto payload = userver::formats::json::MakeObject(
        "id", delivery.id,
        "parcel_id", delivery.parcel_id,
        "sender_id", delivery.sender_id,
        "receiver_id", delivery.receiver_id,
        "tracking_number", delivery.tracking_number,
        "status", delivery.status,
        "from_address", delivery.from_address,
        "to_address", delivery.to_address);
    PublishEvent("delivery.created", "delivery.created", payload);
  }

  void DeliveryEventProducer::PublishDeliveryStatusChanged(const std::string &delivery_id,
                                                           const std::string &old_status,
                                                           const std::string &new_status)
  {
    auto payload = userver::formats::json::MakeObject(
        "delivery_id", delivery_id,
        "old_status", old_status,
        "new_status", new_status);
    PublishEvent("delivery.status_changed", "delivery.status_changed", payload);
  }

} // namespace components