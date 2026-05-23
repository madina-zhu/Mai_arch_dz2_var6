#pragma once

#include <string>
#include <vector>
#include <optional>
#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/redis/client.hpp>
#include <userver/storages/redis/component.hpp>
#include <userver/storages/redis/reply.hpp>

#include "../models/dto.hpp"

namespace components
{

  class RedisCacheComponent final : public userver::components::ComponentBase
  {
  public:
    static constexpr std::string_view kName = "redis-cache-component";

    RedisCacheComponent(const userver::components::ComponentConfig &config,
                        const userver::components::ComponentContext &context);

    // ==================== DELIVERIES BY SENDER ====================
    std::optional<std::vector<models::dto::DeliveryResponse>>
    SearchDeliveriesBySender(int64_t sender_id, int from, int to);

    void SaveDeliveriesSearchResult(
        const std::string &key,
        const std::vector<models::dto::DeliveryResponse> &data);

    void InvalidateSenderDeliveriesCache(int64_t sender_id);

    // ==================== DELIVERIES BY RECEIVER ====================
    std::optional<std::vector<models::dto::DeliveryResponse>>
    SearchDeliveriesByReceiver(int64_t receiver_id, int from, int to);

    void InvalidateReceiverDeliveriesCache(int64_t receiver_id);

    // ==================== DELIVERY BY TRACKING NUMBER ====================
    std::optional<models::dto::DeliveryResponse>
    SearchDeliveryByTrackingNumber(const std::string &tracking_number);

    void SaveDeliveryByTrackingNumber(const std::string &tracking_number,
                                      const models::dto::DeliveryResponse &data);

    // ==================== USER BY LOGIN ====================
    std::optional<models::dto::UserResponse>
    SearchUserByLogin(const std::string &login);

    void SaveUserByLogin(const std::string &login,
                         const models::dto::UserResponse &data);

  private:
    std::shared_ptr<userver::storages::redis::Client> redis_client_;
  };

} // namespace components