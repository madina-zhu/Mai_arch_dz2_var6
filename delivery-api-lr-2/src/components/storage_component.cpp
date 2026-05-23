#include "storage_component.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <userver/logging/log.hpp>

namespace components
{

  StorageComponent::StorageComponent(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : ComponentBase(config, context) {}

  std::string StorageComponent::GetCurrentTimestamp()
  {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
  }

  std::string StorageComponent::GenerateTrackingNumber(int64_t delivery_id)
  {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    std::string suffix;
    for (int i = 0; i < 6; ++i)
    {
      suffix += alphanum[dis(gen)];
    }
    return "DEL-" + std::to_string(delivery_id) + "-" + suffix;
  }

  // ==================== USER OPERATIONS ====================

  int64_t
  StorageComponent::RegisterUser(const models::dto::UserCreateRequest &request,
                                 const std::string &password_hash)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto &[id, data] : users_)
    {
      if (data.request.login == request.login)
      {
        return -1;
      }
    }

    int64_t id = user_id_counter_++;
    users_[id] = {id,
                  request,
                  password_hash,
                  GetCurrentTimestamp()};

    LOG_INFO() << "Registered user with id=" << id << " login=" << request.login;
    return id;
  }

  std::optional<int64_t>
  StorageComponent::VerifyCredentials(const std::string &login,
                                      const std::string &password_plain)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto &[id, data] : users_)
    {
      if (data.request.login == login)
      {
        LOG_INFO() << "Found user " << login << ", stored password: " << data.request.password;
        LOG_INFO() << "Comparing with incoming password: " << password_plain;

        if (data.request.password == password_plain)
        {
          LOG_INFO() << "Password match!";
          return id;
        }
        LOG_INFO() << "Password mismatch!";
        return std::nullopt;
      }
    }
    LOG_INFO() << "User not found: " << login;
    return std::nullopt;
  }

  std::optional<models::dto::UserResponse>
  StorageComponent::GetUserByLogin(const std::string &login)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto &[id, data] : users_)
    {
      if (data.request.login == login)
      {
        return models::dto::UserResponse{data.id,
                                         data.request.login,
                                         data.request.first_name,
                                         data.request.last_name,
                                         data.request.email,
                                         data.created_at};
      }
    }
    return std::nullopt;
  }

  int64_t StorageComponent::CreateUser(const models::dto::UserCreateRequest &request)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto &[id, data] : users_)
    {
      if (data.request.login == request.login)
      {
        return -1;
      }
    }

    int64_t id = user_id_counter_++;
    users_[id] = {id,
                  request,
                  request.password,
                  GetCurrentTimestamp()};

    LOG_INFO() << "Created user with id=" << id << " login=" << request.login << " password=" << request.password;
    return id;
  }

  std::optional<models::dto::UserResponse>
  StorageComponent::GetUserById(int64_t id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(id);
    if (it == users_.end())
    {
      return std::nullopt;
    }

    const auto &data = it->second;
    return models::dto::UserResponse{data.id,
                                     data.request.login,
                                     data.request.first_name,
                                     data.request.last_name,
                                     data.request.email,
                                     data.created_at};
  }

  std::vector<models::dto::UserResponse>
  StorageComponent::SearchUsersByNameMask(const std::string &mask)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<models::dto::UserResponse> result;
    std::string search_pattern = mask;
    if (!search_pattern.empty() && search_pattern.back() == '*')
    {
      search_pattern.pop_back();
    }

    for (const auto &[id, data] : users_)
    {
      std::string full_name =
          data.request.first_name + " " + data.request.last_name;
      if (full_name.find(search_pattern) != std::string::npos)
      {
        result.push_back({data.id, data.request.login, data.request.first_name,
                          data.request.last_name, data.request.email,
                          data.created_at});
      }
    }
    return result;
  }

  // ==================== PARCEL OPERATIONS ====================

  int64_t StorageComponent::CreateParcel(
      const models::dto::ParcelCreateRequest &request)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (users_.find(request.sender_id) == users_.end())
    {
      return -1;
    }

    int64_t id = parcel_id_counter_++;
    parcels_[id] = {id, request, "created", GetCurrentTimestamp()};

    LOG_INFO() << "Created parcel with id=" << id << " sender_id=" << request.sender_id;
    return id;
  }

  std::optional<models::dto::ParcelResponse>
  StorageComponent::GetParcelById(int64_t id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = parcels_.find(id);
    if (it == parcels_.end())
    {
      return std::nullopt;
    }

    const auto &data = it->second;
    return models::dto::ParcelResponse{
        data.id, data.request.sender_id, data.request.weight,
        data.request.dimensions, data.request.declared_value,
        data.request.description, data.status, data.created_at};
  }

  std::vector<models::dto::ParcelResponse>
  StorageComponent::GetUserParcels(int64_t user_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<models::dto::ParcelResponse> result;
    for (const auto &[id, data] : parcels_)
    {
      if (data.request.sender_id == user_id)
      {
        result.push_back({data.id, data.request.sender_id, data.request.weight,
                          data.request.dimensions, data.request.declared_value,
                          data.request.description, data.status, data.created_at});
      }
    }
    return result;
  }

  bool StorageComponent::UpdateParcelStatus(int64_t id, const std::string &status)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = parcels_.find(id);
    if (it == parcels_.end())
    {
      return false;
    }

    it->second.status = status;
    LOG_INFO() << "Updated parcel id=" << id << " status=" << status;
    return true;
  }

  // ==================== DELIVERY OPERATIONS ====================

  int64_t StorageComponent::CreateDelivery(
      const models::dto::DeliveryCreateRequest &request)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto parcel_it = parcels_.find(request.parcel_id);
    if (parcel_it == parcels_.end())
    {
      return -1;
    }

    if (users_.find(request.receiver_id) == users_.end())
    {
      return -2;
    }

    int64_t id = delivery_id_counter_++;
    std::string tracking_number = GenerateTrackingNumber(id);

    deliveries_[id] = {id, request, tracking_number, "pending",
                       GetCurrentTimestamp(), std::nullopt};

    parcel_it->second.status = "assigned";

    LOG_INFO() << "Created delivery with id=" << id
               << " tracking=" << tracking_number;
    return id;
  }

  std::optional<models::dto::DeliveryResponse>
  StorageComponent::GetDeliveryById(int64_t id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = deliveries_.find(id);
    if (it == deliveries_.end())
    {
      return std::nullopt;
    }

    const auto &data = it->second;

    int64_t sender_id = 0;
    auto parcel_it = parcels_.find(data.request.parcel_id);
    if (parcel_it != parcels_.end())
    {
      sender_id = parcel_it->second.request.sender_id;
    }

    return models::dto::DeliveryResponse{
        data.id, data.request.parcel_id, sender_id, data.request.receiver_id,
        data.tracking_number, data.status, data.request.from_address,
        data.request.to_address, data.created_at, data.delivered_at};
  }

  std::vector<models::dto::DeliveryResponse>
  StorageComponent::GetDeliveriesBySender(int64_t sender_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<models::dto::DeliveryResponse> result;
    for (const auto &[id, data] : deliveries_)
    {
      auto parcel_it = parcels_.find(data.request.parcel_id);
      if (parcel_it != parcels_.end() &&
          parcel_it->second.request.sender_id == sender_id)
      {
        result.push_back({data.id, data.request.parcel_id, sender_id,
                          data.request.receiver_id, data.tracking_number,
                          data.status, data.request.from_address,
                          data.request.to_address, data.created_at,
                          data.delivered_at});
      }
    }
    return result;
  }

  std::vector<models::dto::DeliveryResponse>
  StorageComponent::GetDeliveriesByReceiver(int64_t receiver_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<models::dto::DeliveryResponse> result;
    for (const auto &[id, data] : deliveries_)
    {
      if (data.request.receiver_id == receiver_id)
      {
        auto parcel_it = parcels_.find(data.request.parcel_id);
        int64_t sender_id = (parcel_it != parcels_.end())
                                ? parcel_it->second.request.sender_id
                                : 0;

        result.push_back({data.id, data.request.parcel_id, sender_id,
                          data.request.receiver_id, data.tracking_number,
                          data.status, data.request.from_address,
                          data.request.to_address, data.created_at,
                          data.delivered_at});
      }
    }
    return result;
  }

  std::optional<models::dto::DeliveryResponse>
  StorageComponent::GetDeliveryByTrackingNumber(const std::string &tracking_number)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto &[id, data] : deliveries_)
    {
      if (data.tracking_number == tracking_number)
      {
        auto parcel_it = parcels_.find(data.request.parcel_id);
        int64_t sender_id = (parcel_it != parcels_.end())
                                ? parcel_it->second.request.sender_id
                                : 0;

        return models::dto::DeliveryResponse{
            data.id, data.request.parcel_id, sender_id, data.request.receiver_id,
            data.tracking_number, data.status, data.request.from_address,
            data.request.to_address, data.created_at, data.delivered_at};
      }
    }
    return std::nullopt;
  }

  bool StorageComponent::UpdateDeliveryStatus(int64_t id, const std::string &status)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = deliveries_.find(id);
    if (it == deliveries_.end())
    {
      return false;
    }

    it->second.status = status;
    if (status == "delivered")
    {
      it->second.delivered_at = GetCurrentTimestamp();
    }

    LOG_INFO() << "Updated delivery id=" << id << " status=" << status;
    return true;
  }

} // namespace components