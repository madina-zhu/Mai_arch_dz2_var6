#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <vector>

#include "../models/dto.hpp"

namespace components
{

  class StorageComponent : public userver::components::ComponentBase
  {
  public:
    static constexpr std::string_view kName = "storage-component";

    explicit StorageComponent(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context);

    // ==================== USER OPERATIONS ====================
    int64_t RegisterUser(const models::dto::UserCreateRequest &request,
                         const std::string &password_hash);
    std::optional<int64_t> VerifyCredentials(const std::string &login,
                                             const std::string &password_plain);
    std::optional<models::dto::UserResponse>
    GetUserByLogin(const std::string &login);
    int64_t CreateUser(const models::dto::UserCreateRequest &request);
    std::optional<models::dto::UserResponse> GetUserById(int64_t id);
    std::vector<models::dto::UserResponse>
    SearchUsersByNameMask(const std::string &mask);

    // ==================== PARCEL OPERATIONS ====================
    int64_t CreateParcel(const models::dto::ParcelCreateRequest &request);
    std::optional<models::dto::ParcelResponse> GetParcelById(int64_t id);
    std::vector<models::dto::ParcelResponse> GetUserParcels(int64_t user_id);
    bool UpdateParcelStatus(int64_t id, const std::string &status);

    // ==================== DELIVERY OPERATIONS ====================
    int64_t CreateDelivery(const models::dto::DeliveryCreateRequest &request);
    std::optional<models::dto::DeliveryResponse> GetDeliveryById(int64_t id);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesBySender(int64_t sender_id);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesByReceiver(int64_t receiver_id);
    std::optional<models::dto::DeliveryResponse>
    GetDeliveryByTrackingNumber(const std::string &tracking_number);
    bool UpdateDeliveryStatus(int64_t id, const std::string &status);

  private:
    struct UserData
    {
      int64_t id;
      models::dto::UserCreateRequest request;
      std::string password_hash;
      std::string created_at;
    };

    struct ParcelData
    {
      int64_t id;
      models::dto::ParcelCreateRequest request;
      std::string status;
      std::string created_at;
    };

    struct DeliveryData
    {
      int64_t id;
      models::dto::DeliveryCreateRequest request;
      std::string tracking_number;
      std::string status;
      std::string created_at;
      std::optional<std::string> delivered_at;
    };

    std::map<int64_t, UserData> users_;
    std::map<int64_t, ParcelData> parcels_;
    std::map<int64_t, DeliveryData> deliveries_;
    std::map<std::string, int64_t> tracking_index_;

    std::atomic<int64_t> user_id_counter_{1};
    std::atomic<int64_t> parcel_id_counter_{1};
    std::atomic<int64_t> delivery_id_counter_{1};

    std::mutex mutex_;

    std::string GetCurrentTimestamp();
    std::string GenerateTrackingNumber(int64_t delivery_id);
  };

} // namespace components