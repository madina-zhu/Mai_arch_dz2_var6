#pragma once

#include <string>
#include <userver/components/component_base.hpp>
#include <userver/formats/json.hpp>
#include <userver/urabbitmq/client.hpp>
#include <userver/urabbitmq/component.hpp>
#include <userver/urabbitmq/typedefs.hpp>

#include "../models/dto.hpp"

namespace components
{

  class DeliveryEventProducer final : public userver::components::LoggableComponentBase
  {
  public:
    static constexpr std::string_view kName = "delivery-event-producer";

    DeliveryEventProducer(const userver::components::ComponentConfig &config,
                          const userver::components::ComponentContext &context);

    void PublishUserRegistered(const models::dto::UserCreateRequest &user, int64_t user_id);
    void PublishParcelCreated(const std::string &id, const models::dto::ParcelCreateRequest &parcel);
    void PublishDeliveryCreated(const models::dto::DeliveryResponse &delivery);
    void PublishDeliveryStatusChanged(const std::string &delivery_id,
                                      const std::string &old_status,
                                      const std::string &new_status);

  private:
    void PublishEvent(const std::string &event_type,
                      const std::string &routing_key,
                      const userver::formats::json::Value &payload);

    std::shared_ptr<userver::urabbitmq::Client> publisher_;
    userver::urabbitmq::Exchange exchange_;
  };

} // namespace components