#include "redis_cache_component.hpp"
#include <chrono>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/redis/command_control.hpp>

namespace components
{

  namespace
  {

    constexpr int kUserTtl = 300;
    constexpr int kDeliverySearchTtl = 60;
    constexpr int kDeliveryByTrackingTtl = 300;

    std::string MakeSenderDeliveriesKey(int64_t sender_id, int from, int to)
    {
      return "deliveries:sender:" + std::to_string(sender_id) + ":" +
             std::to_string(from) + ":" + std::to_string(to);
    }

    std::string MakeReceiverDeliveriesKey(int64_t receiver_id, int from, int to)
    {
      return "deliveries:receiver:" + std::to_string(receiver_id) + ":" +
             std::to_string(from) + ":" + std::to_string(to);
    }

    std::string MakeTrackingKey(const std::string &tracking_number)
    {
      return "delivery:tracking:" + tracking_number;
    }

    std::string MakeUserLoginKey(const std::string &login)
    {
      return "user:login:" + login;
    }

  } // namespace

  RedisCacheComponent::RedisCacheComponent(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : ComponentBase(config, context),
        redis_client_(
            context.FindComponent<userver::components::Redis>("redis").GetClient("main")) {}

  // ==================== DELIVERIES BY SENDER ====================

  std::optional<std::vector<models::dto::DeliveryResponse>>
  RedisCacheComponent::SearchDeliveriesBySender(int64_t sender_id, int from, int to)
  {
    const auto key = MakeSenderDeliveriesKey(sender_id, from, to);
    auto reply = redis_client_->Get(key, userver::storages::redis::CommandControl{}).Get();

    if (!reply.has_value())
    {
      return std::nullopt;
    }

    try
    {
      auto json = userver::formats::json::FromString(reply.value());
      std::vector<models::dto::DeliveryResponse> result;
      for (const auto &item : json)
      {
        result.push_back(item.As<models::dto::DeliveryResponse>());
      }
      return result;
    }
    catch (const std::exception &e)
    {
      LOG_WARNING() << "Failed to parse cached deliveries: " << e.what();
      return std::nullopt;
    }
  }

  void RedisCacheComponent::SaveDeliveriesSearchResult(
      const std::string &key,
      const std::vector<models::dto::DeliveryResponse> &data)
  {
    userver::formats::json::ValueBuilder builder;
    for (const auto &item : data)
    {
      builder.PushBack(userver::formats::json::ValueBuilder(item).ExtractValue());
    }
    redis_client_->Setex(key, std::chrono::seconds(kDeliverySearchTtl),
                         userver::formats::json::ToString(builder.ExtractValue()),
                         userver::storages::redis::CommandControl{})
        .Get();
  }

  void RedisCacheComponent::InvalidateSenderDeliveriesCache(int64_t sender_id)
  {
    const auto pattern = "deliveries:sender:" + std::to_string(sender_id) + ":*";
    std::vector<std::string> keys_to_delete;
    size_t cursor = 0;

    do
    {
      auto scan_result =
          redis_client_
              ->Scan(cursor,
                     userver::storages::redis::ScanOptionsGeneric{
                         userver::storages::redis::Match{pattern},
                         userver::storages::redis::Count{100}},
                     userver::storages::redis::CommandControl{})
              .GetAll();
      for (const auto &key : scan_result)
      {
        keys_to_delete.push_back(key);
      }
    } while (cursor != 0);

    if (!keys_to_delete.empty())
    {
      redis_client_->Del(keys_to_delete, userver::storages::redis::CommandControl{}).Get();
    }
  }

  // ==================== DELIVERIES BY RECEIVER ====================

  std::optional<std::vector<models::dto::DeliveryResponse>>
  RedisCacheComponent::SearchDeliveriesByReceiver(int64_t receiver_id, int from, int to)
  {
    const auto key = MakeReceiverDeliveriesKey(receiver_id, from, to);
    auto reply = redis_client_->Get(key, userver::storages::redis::CommandControl{}).Get();

    if (!reply.has_value())
    {
      return std::nullopt;
    }

    try
    {
      auto json = userver::formats::json::FromString(reply.value());
      std::vector<models::dto::DeliveryResponse> result;
      for (const auto &item : json)
      {
        result.push_back(item.As<models::dto::DeliveryResponse>());
      }
      return result;
    }
    catch (const std::exception &e)
    {
      LOG_WARNING() << "Failed to parse cached deliveries: " << e.what();
      return std::nullopt;
    }
  }

  void RedisCacheComponent::InvalidateReceiverDeliveriesCache(int64_t receiver_id)
  {
    const auto pattern = "deliveries:receiver:" + std::to_string(receiver_id) + ":*";
    std::vector<std::string> keys_to_delete;
    size_t cursor = 0;

    do
    {
      auto scan_result =
          redis_client_
              ->Scan(cursor,
                     userver::storages::redis::ScanOptionsGeneric{
                         userver::storages::redis::Match{pattern},
                         userver::storages::redis::Count{100}},
                     userver::storages::redis::CommandControl{})
              .GetAll();
      for (const auto &key : scan_result)
      {
        keys_to_delete.push_back(key);
      }
    } while (cursor != 0);

    if (!keys_to_delete.empty())
    {
      redis_client_->Del(keys_to_delete, userver::storages::redis::CommandControl{}).Get();
    }
  }

  // ==================== DELIVERY BY TRACKING NUMBER ====================

  std::optional<models::dto::DeliveryResponse>
  RedisCacheComponent::SearchDeliveryByTrackingNumber(const std::string &tracking_number)
  {
    const auto key = MakeTrackingKey(tracking_number);
    auto reply = redis_client_->Get(key, userver::storages::redis::CommandControl{}).Get();

    if (!reply.has_value())
    {
      return std::nullopt;
    }

    try
    {
      auto json = userver::formats::json::FromString(reply.value());
      return json.As<models::dto::DeliveryResponse>();
    }
    catch (const std::exception &e)
    {
      LOG_WARNING() << "Failed to parse cached delivery: " << e.what();
      return std::nullopt;
    }
  }

  void RedisCacheComponent::SaveDeliveryByTrackingNumber(
      const std::string &tracking_number,
      const models::dto::DeliveryResponse &data)
  {
    const auto key = MakeTrackingKey(tracking_number);
    redis_client_->Setex(key, std::chrono::seconds(kDeliveryByTrackingTtl),
                         userver::formats::json::ToString(
                             userver::formats::json::ValueBuilder(data).ExtractValue()),
                         userver::storages::redis::CommandControl{})
        .Get();
  }

  // ==================== USER BY LOGIN ====================

  std::optional<models::dto::UserResponse>
  RedisCacheComponent::SearchUserByLogin(const std::string &login)
  {
    const auto key = MakeUserLoginKey(login);
    auto reply = redis_client_->Get(key, userver::storages::redis::CommandControl{}).Get();

    if (!reply.has_value())
    {
      return std::nullopt;
    }

    try
    {
      auto json = userver::formats::json::FromString(reply.value());
      return json.As<models::dto::UserResponse>();
    }
    catch (const std::exception &e)
    {
      LOG_WARNING() << "Failed to parse cached user: " << e.what();
      return std::nullopt;
    }
  }

  void RedisCacheComponent::SaveUserByLogin(const std::string &login,
                                            const models::dto::UserResponse &data)
  {
    const auto key = MakeUserLoginKey(login);
    redis_client_->Setex(key, std::chrono::seconds(kUserTtl),
                         userver::formats::json::ToString(
                             userver::formats::json::ValueBuilder(data).ExtractValue()),
                         userver::storages::redis::CommandControl{})
        .Get();
  }

} // namespace components