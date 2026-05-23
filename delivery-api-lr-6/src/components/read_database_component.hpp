#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <userver/components/component_base.hpp>

#include "../models/dto.hpp"

namespace components
{

  class ReadDatabaseComponent : public userver::components::ComponentBase
  {
  public:
    static constexpr std::string_view kName = "read-database-component";

    explicit ReadDatabaseComponent(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context);

    // ==================== PARCEL OPERATIONS ====================
    std::optional<models::dto::ParcelResponse> GetParcelById(const std::string &parcel_id);
    std::vector<models::dto::ParcelResponse> GetParcelsBySender(int64_t sender_id, int to, int from);

    // ==================== DELIVERY OPERATIONS ====================
    std::optional<models::dto::DeliveryResponse> GetDeliveryById(const std::string &delivery_id);
    std::optional<models::dto::DeliveryResponse> GetDeliveryByTrackingNumber(const std::string &tracking_number);
    std::vector<models::dto::DeliveryResponse> GetDeliveriesBySender(int64_t sender_id, int to, int from);
    std::vector<models::dto::DeliveryResponse> GetDeliveriesByReceiver(int64_t receiver_id, int to, int from);

    // ==================== USER OPERATIONS ====================
    std::optional<models::dto::UserResponse> GetUserByLogin(const std::string &login, int from, int to);
    std::optional<models::dto::UserResponse> GetUserById(int64_t id);
    std::vector<models::dto::UserResponse> SearchUsersByNameMask(const std::string &mask, int from, int to);
    std::optional<int64_t> VerifyCredentials(const std::string &login, const std::string &password_hash);

    // ==================== UPDATE METHODS (called by consumer) ====================
    void OnUserCreated(int64_t user_id, const models::dto::UserCreateRequest &user, const std::string &password_hash);
    void OnParcelCreated(const std::string &parcel_id, const models::dto::ParcelCreateRequest &parcel);
    void OnDeliveryCreated(const models::dto::DeliveryResponse &delivery);
    void OnDeliveryStatusChanged(const std::string &delivery_id, const std::string &new_status);

  private:
    struct StoredUser
    {
      models::dto::UserResponse data;
      std::string password_hash;
    };

    struct StoredParcel
    {
      models::dto::ParcelResponse data;
    };

    struct StoredDelivery
    {
      models::dto::DeliveryResponse data;
    };

    std::unordered_map<int64_t, StoredUser> users_;
    std::unordered_map<std::string, int64_t> user_login_index_;
    std::unordered_map<std::string, StoredParcel> parcels_;
    std::unordered_map<std::string, StoredDelivery> deliveries_;
    std::unordered_map<std::string, std::string> tracking_to_id_;

    mutable std::mutex mutex_;
    std::atomic<int64_t> user_id_counter_{1000};

    std::string GenerateId();
    int64_t GenerateUserId();

    template <typename T>
    std::vector<T> ApplyPagination(const std::vector<T> &items, int from, int to) const;
  };

} // namespace components