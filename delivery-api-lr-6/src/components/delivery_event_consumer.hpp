#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <userver/formats/json/value.hpp>
#include <userver/urabbitmq/consumer_component_base.hpp>

#include "../models/dto.hpp"
#include "read_database_component.hpp"

namespace components
{

  class DeliveryEventConsumer final : public userver::urabbitmq::ConsumerComponentBase
  {
  public:
    static constexpr std::string_view kName = "delivery-event-consumer";

    DeliveryEventConsumer(const userver::components::ComponentConfig &config,
                          const userver::components::ComponentContext &context);

  protected:
    void Process(std::string message) override;

  private:
    void HandleUserRegistered(const userver::formats::json::Value &payload);
    void HandleParcelCreated(const userver::formats::json::Value &payload);
    void HandleDeliveryCreated(const userver::formats::json::Value &payload);
    void HandleDeliveryStatusChanged(const userver::formats::json::Value &payload);

    std::unordered_map<std::string,
                       std::function<void(const userver::formats::json::Value &)>>
        handlers_;

    ReadDatabaseComponent &read_db_;
  };

} // namespace components