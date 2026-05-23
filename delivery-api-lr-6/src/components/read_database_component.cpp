#include "read_database_component.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <userver/logging/log.hpp>
#include <userver/utils/uuid4.hpp>

namespace components
{

  ReadDatabaseComponent::ReadDatabaseComponent(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : userver::components::ComponentBase(config, context)
  {
    LOG_INFO() << "ReadDatabaseComponent (In-Memory) initialized";
  }

  std::string ReadDatabaseComponent::GenerateId()
  {
    return userver::utils::generators::GenerateUuid();
  }

  int64_t ReadDatabaseComponent::GenerateUserId()
  {
    return user_id_counter_.fetch_add(1);
  }

  // ==================== USER UPDATE METHODS ====================

  void ReadDatabaseComponent::OnUserCreated(int64_t user_id,
                                            const models::dto::UserCreateRequest &user,
                                            const std::string &password_hash)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    StoredUser stored;
    stored.data.id = user_id;
    stored.data.login = user.login;
    stored.data.first_name = user.first_name;
    stored.data.last_name = user.last_name;
    stored.data.email = user.email;
    stored.data.created_at = userver::storages::postgres::TimePointTz{std::chrono::system_clock::now()};
    stored.password_hash = password_hash;

    users_[user_id] = std::move(stored);
    user_login_index_[user.login] = user_id;

    LOG_INFO() << "ReadDB: User created: id=" << user_id << ", login=" << user.login;
  }

  // ==================== PARCEL UPDATE METHODS ====================

  void ReadDatabaseComponent::OnParcelCreated(const std::string &parcel_id,
                                              const models::dto::ParcelCreateRequest &parcel)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    StoredParcel stored;
    stored.data.id = parcel_id;
    stored.data.sender_id = parcel.sender_id;
    stored.data.weight = parcel.weight;
    stored.data.dimensions = parcel.dimensions;
    stored.data.declared_value = parcel.declared_value;
    stored.data.description = parcel.description;
    stored.data.status = "created";
    stored.data.created_at = std::chrono::system_clock::now();

    parcels_[parcel_id] = std::move(stored);

    LOG_INFO() << "ReadDB: Parcel created: id=" << parcel_id << ", sender=" << parcel.sender_id;
  }

  // ==================== DELIVERY UPDATE METHODS ====================

  void ReadDatabaseComponent::OnDeliveryCreated(const models::dto::DeliveryResponse &delivery)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    StoredDelivery stored;
    stored.data = delivery;
    deliveries_[delivery.id] = std::move(stored);
    tracking_to_id_[delivery.tracking_number] = delivery.id;

    LOG_INFO() << "ReadDB: Delivery created: id=" << delivery.id
               << ", tracking=" << delivery.tracking_number;
  }

  void ReadDatabaseComponent::OnDeliveryStatusChanged(const std::string &delivery_id,
                                                      const std::string &new_status)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = deliveries_.find(delivery_id);
    if (it != deliveries_.end())
    {
      it->second.data.status = new_status;
      if (new_status == "delivered")
      {
        it->second.data.delivered_at = std::chrono::system_clock::now();
      }
      LOG_INFO() << "ReadDB: Delivery status changed: id=" << delivery_id
                 << " -> " << new_status;
    }
  }

  // ==================== PARCEL READ OPERATIONS ====================

  std::optional<models::dto::ParcelResponse>
  ReadDatabaseComponent::GetParcelById(const std::string &parcel_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end())
    {
      return it->second.data;
    }
    return std::nullopt;
  }

  std::vector<models::dto::ParcelResponse>
  ReadDatabaseComponent::GetParcelsBySender(int64_t sender_id, int to, int from)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::dto::ParcelResponse> result;

    for (const auto &[id, stored] : parcels_)
    {
      if (stored.data.sender_id == sender_id)
      {
        result.push_back(stored.data);
      }
    }

    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              { return a.created_at > b.created_at; });

    return ApplyPagination(result, from, to);
  }

  // ==================== DELIVERY READ OPERATIONS ====================

  std::optional<models::dto::DeliveryResponse>
  ReadDatabaseComponent::GetDeliveryById(const std::string &delivery_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = deliveries_.find(delivery_id);
    if (it != deliveries_.end())
    {
      return it->second.data;
    }
    return std::nullopt;
  }

  std::optional<models::dto::DeliveryResponse>
  ReadDatabaseComponent::GetDeliveryByTrackingNumber(const std::string &tracking_number)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tracking_to_id_.find(tracking_number);
    if (it != tracking_to_id_.end())
    {
      auto delivery_it = deliveries_.find(it->second);
      if (delivery_it != deliveries_.end())
      {
        return delivery_it->second.data;
      }
    }
    return std::nullopt;
  }

  std::vector<models::dto::DeliveryResponse>
  ReadDatabaseComponent::GetDeliveriesBySender(int64_t sender_id, int to, int from)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::dto::DeliveryResponse> result;

    for (const auto &[id, stored] : deliveries_)
    {
      if (stored.data.sender_id == sender_id)
      {
        result.push_back(stored.data);
      }
    }

    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              { return a.created_at > b.created_at; });

    return ApplyPagination(result, from, to);
  }

  std::vector<models::dto::DeliveryResponse>
  ReadDatabaseComponent::GetDeliveriesByReceiver(int64_t receiver_id, int to, int from)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::dto::DeliveryResponse> result;

    for (const auto &[id, stored] : deliveries_)
    {
      if (stored.data.receiver_id == receiver_id)
      {
        result.push_back(stored.data);
      }
    }

    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              { return a.created_at > b.created_at; });

    return ApplyPagination(result, from, to);
  }

  // ==================== USER READ OPERATIONS ====================

  std::optional<models::dto::UserResponse>
  ReadDatabaseComponent::GetUserByLogin(const std::string &login, int from, int to)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_login_index_.find(login);
    if (it != user_login_index_.end())
    {
      if (from == 1 && to >= 1)
      {
        auto user_it = users_.find(it->second);
        if (user_it != users_.end())
        {
          return user_it->second.data;
        }
      }
    }
    return std::nullopt;
  }

  std::optional<models::dto::UserResponse>
  ReadDatabaseComponent::GetUserById(int64_t id)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(id);
    if (it != users_.end())
    {
      return it->second.data;
    }
    return std::nullopt;
  }

  std::vector<models::dto::UserResponse>
  ReadDatabaseComponent::SearchUsersByNameMask(const std::string &mask, int from, int to)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::dto::UserResponse> result;

    std::string search_pattern = mask;
    std::replace(search_pattern.begin(), search_pattern.end(), '*', '%');
    search_pattern.pop_back(); // убираем % в конце для простого поиска

    for (const auto &[id, stored] : users_)
    {
      std::string full_name = stored.data.first_name + " " + stored.data.last_name;
      if (full_name.find(search_pattern) != std::string::npos)
      {
        result.push_back(stored.data);
      }
    }

    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              { return a.id < b.id; });

    return ApplyPagination(result, from, to);
  }

  std::optional<int64_t>
  ReadDatabaseComponent::VerifyCredentials(const std::string &login, const std::string &password_hash)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_login_index_.find(login);
    if (it != user_login_index_.end())
    {
      auto user_it = users_.find(it->second);
      if (user_it != users_.end() && user_it->second.password_hash == password_hash)
      {
        return user_it->second.data.id;
      }
    }
    return std::nullopt;
  }

  // ==================== HELPER ====================

  template <typename T>
  std::vector<T>
  ReadDatabaseComponent::ApplyPagination(const std::vector<T> &items, int from, int to) const
  {
    if (items.empty())
      return {};

    int start = std::max(1, from);
    int end = std::min(static_cast<int>(items.size()), to);

    if (start > end || start > static_cast<int>(items.size()))
    {
      return {};
    }

    return std::vector<T>(items.begin() + start - 1, items.begin() + end);
  }

} // namespace components