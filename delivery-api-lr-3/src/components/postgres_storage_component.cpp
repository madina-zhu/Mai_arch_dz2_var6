#include "postgres_storage_component.hpp"
#include <cstdint>
#include <random>
#include <string>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/postgres.hpp>
#include <userver/storages/postgres/row.hpp>
#include <userver/storages/postgres/sql_state.hpp>

namespace components
{

  using userver::storages::postgres::ClusterHostType;

  std::string PostgresStorageComponent::GenerateTrackingNumber(int64_t delivery_id)
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

  PostgresStorageComponent::PostgresStorageComponent(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : ComponentBase(config, context),
        cluster_(
            context
                .FindComponent<userver::components::Postgres>("db-postgresql")
                .GetCluster()) {}

  // ==================== USER OPERATIONS ====================

  int64_t PostgresStorageComponent::RegisterUser(
      const models::dto::UserCreateRequest &request,
      const std::string &password_hash)
  {

    const std::string query = R"(
    INSERT INTO users (login, password_hash, first_name, last_name, email, created_at)
    VALUES ($1, $2, $3, $4, $5, NOW())
    RETURNING id
  )";

    try
    {
      auto result = cluster_->Execute(
          ClusterHostType::kMaster, query, request.login, password_hash,
          request.first_name, request.last_name, request.email);
      return result.AsSingleRow<int64_t>();
    }
    catch (const userver::storages::postgres::UniqueViolation &e)
    {
      LOG_WARNING() << "Registration failed: user with login '" << request.login
                    << "' and/or email '" << request.email << "' already exists.";
      return UNIQUE_VIOLATION;
    }
    catch (const userver::storages::postgres::CheckViolation &e)
    {
      LOG_WARNING() << "Registration failed: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
    catch (const userver::storages::postgres::DataException &e)
    {
      LOG_WARNING() << "Registration failed: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
  }

  std::optional<int64_t> PostgresStorageComponent::VerifyCredentials(
      const std::string &login, const std::string &password_plain)
  {

    struct UserAuthData
    {
      int64_t id;
      std::string password_hash;
    };

    const std::string query =
        "SELECT id, password_hash FROM users WHERE login = $1";

    auto opt_row = cluster_->Execute(ClusterHostType::kMaster, query, login)
                       .AsOptionalSingleRow<UserAuthData>(
                           userver::storages::postgres::kRowTag);

    if (!opt_row)
    {
      return std::nullopt;
    }

    const auto &data = opt_row.value();

    if (data.password_hash == password_plain)
    {
      return data.id;
    }

    return std::nullopt;
  }

  std::optional<models::dto::UserResponse> PostgresStorageComponent::GetUserByLogin(
      const std::string &login, int from, int to)
  {
    const std::string query = R"(
    SELECT id, login, first_name, last_name, email, created_at 
    FROM users 
    WHERE login = $1
    LIMIT $2
    OFFSET $3
  )";

    auto opt_row = cluster_
                       ->Execute(ClusterHostType::kMaster, query, login,
                                 to - from + 1, from - 1)
                       .AsOptionalSingleRow<models::dto::UserResponse>(
                           userver::storages::postgres::kRowTag);

    if (!opt_row)
    {
      return std::nullopt;
    }

    return opt_row.value();
  }

  int64_t PostgresStorageComponent::CreateUser(
      const models::dto::UserCreateRequest &request,
      const std::string &password_hash)
  {
    return RegisterUser(request, password_hash);
  }

  std::optional<models::dto::UserResponse> PostgresStorageComponent::GetUserById(
      int64_t id)
  {
    const std::string query = R"(
    SELECT id, login, first_name, last_name, email, created_at 
    FROM users 
    WHERE id = $1
  )";

    auto opt_row = cluster_->Execute(ClusterHostType::kMaster, query, id)
                       .AsOptionalSingleRow<models::dto::UserResponse>(
                           userver::storages::postgres::kRowTag);

    if (!opt_row)
    {
      return std::nullopt;
    }

    return opt_row.value();
  }

  std::vector<models::dto::UserResponse>
  PostgresStorageComponent::SearchUsersByNameMask(const std::string &mask, int from,
                                                  int to)
  {
    std::string search_pattern = mask;
    std::replace(search_pattern.begin(), search_pattern.end(), '*', '%');

    const std::string query = R"(
    SELECT id, login, first_name, last_name, email, created_at 
    FROM users 
    WHERE (first_name || ' ' || last_name) LIKE $1
    LIMIT $2
    OFFSET $3
  )";

    auto rows = cluster_->Execute(ClusterHostType::kMaster, query,
                                  search_pattern, to - from + 1, from - 1);

    std::vector<models::dto::UserResponse> result;
    result.reserve(rows.Size());

    for (const auto &row : rows)
    {
      result.push_back(
          {row["id"].As<int64_t>(), row["login"].As<std::string>(),
           row["first_name"].As<std::string>(),
           row["last_name"].As<std::string>(), row["email"].As<std::string>(),
           row["created_at"].As<userver::storages::postgres::TimePointTz>()});
    }

    return result;
  }

  // ==================== PARCEL OPERATIONS ====================

  int64_t PostgresStorageComponent::CreateParcel(
      const models::dto::ParcelCreateRequest &request)
  {

    const std::string query = R"(
    INSERT INTO parcels (sender_id, weight, dimensions, declared_value, description, status, created_at)
    VALUES ($1, $2, $3, $4, $5, 'created', NOW())
    RETURNING id
  )";

    try
    {
      auto result = cluster_->Execute(
          ClusterHostType::kMaster, query, request.sender_id, request.weight,
          request.dimensions, request.declared_value, request.description);
      return result.AsSingleRow<int64_t>();
    }
    catch (const userver::storages::postgres::CheckViolation &e)
    {
      LOG_WARNING() << "Failed to create parcel: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
    catch (const userver::storages::postgres::DataException &e)
    {
      LOG_WARNING() << "Failed to create parcel: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to create parcel: " << e.what();
      return DATA_VIOLATION;
    }
  }

  std::optional<models::dto::ParcelResponse>
  PostgresStorageComponent::GetParcelById(int64_t id)
  {
    const std::string query = R"(
    SELECT id, sender_id, weight, dimensions, declared_value, description, status, created_at 
    FROM parcels 
    WHERE id = $1
  )";

    auto row_opt = cluster_->Execute(ClusterHostType::kMaster, query, id)
                       .AsOptionalSingleRow<models::dto::ParcelResponse>(
                           userver::storages::postgres::kRowTag);

    if (!row_opt)
    {
      return std::nullopt;
    }

    return row_opt.value();
  }

  std::vector<models::dto::ParcelResponse>
  PostgresStorageComponent::GetUserParcels(int64_t user_id, int from, int to)
  {
    const std::string query = R"(
    SELECT id, sender_id, weight, dimensions, declared_value, description, status, created_at 
    FROM parcels 
    WHERE sender_id = $1
    ORDER BY created_at DESC
    LIMIT $2
    OFFSET $3
  )";

    auto rows = cluster_->Execute(ClusterHostType::kMaster, query, user_id,
                                  to - from + 1, from - 1);

    std::vector<models::dto::ParcelResponse> result;
    result.reserve(rows.Size());

    for (const auto &row : rows)
    {
      result.push_back(
          {row["id"].As<int64_t>(), row["sender_id"].As<int64_t>(),
           row["weight"].As<double>(), row["dimensions"].As<std::string>(),
           row["declared_value"].As<double>(), row["description"].As<std::string>(),
           row["status"].As<std::string>(),
           row["created_at"].As<userver::storages::postgres::TimePointTz>()});
    }
    return result;
  }

  bool PostgresStorageComponent::UpdateParcelStatus(int64_t id, const std::string &status)
  {
    const std::string query = R"(
    UPDATE parcels 
    SET status = $1 
    WHERE id = $2
  )";
    auto result = cluster_->Execute(ClusterHostType::kMaster, query, status, id);
    return result.RowsAffected() > 0;
  }

  // ==================== DELIVERY OPERATIONS ====================

  int64_t PostgresStorageComponent::CreateDelivery(
      const models::dto::DeliveryCreateRequest &request)
  {

    std::string tracking_number = GenerateTrackingNumber(0);

    const std::string query = R"(
    INSERT INTO deliveries (parcel_id, sender_id, receiver_id, tracking_number, status, from_address, to_address, created_at)
    VALUES ($1, $2, $3, $4, 'pending', $5, $6, NOW())
    RETURNING id
  )";

    try
    {
      auto result = cluster_->Execute(
          ClusterHostType::kMaster, query, request.parcel_id, request.sender_id,
          request.receiver_id, tracking_number, request.from_address, request.to_address);

      int64_t id = result.AsSingleRow<int64_t>();

      std::string new_tracking_number = GenerateTrackingNumber(id);
      const std::string update_query = R"(
      UPDATE deliveries 
      SET tracking_number = $1 
      WHERE id = $2
    )";
      cluster_->Execute(ClusterHostType::kMaster, update_query, new_tracking_number, id);

      // Update parcel status
      UpdateParcelStatus(request.parcel_id, "assigned");

      return id;
    }
    catch (const userver::storages::postgres::CheckViolation &e)
    {
      LOG_WARNING() << "Failed to create delivery: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
    catch (const userver::storages::postgres::DataException &e)
    {
      LOG_WARNING() << "Failed to create delivery: data constraints were not matched.";
      return CONSTRAINT_VIOLATION;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to create delivery: " << e.what();
      return DATA_VIOLATION;
    }
  }

  std::optional<models::dto::DeliveryResponse>
  PostgresStorageComponent::GetDeliveryById(int64_t id)
  {
    const std::string query = R"(
    SELECT id, parcel_id, sender_id, receiver_id, tracking_number, 
           status, from_address, to_address, created_at, delivered_at 
    FROM deliveries 
    WHERE id = $1
  )";

    auto row_opt = cluster_->Execute(ClusterHostType::kMaster, query, id)
                       .AsOptionalSingleRow<models::dto::DeliveryResponse>(
                           userver::storages::postgres::kRowTag);

    if (!row_opt)
    {
      return std::nullopt;
    }

    return row_opt.value();
  }

  std::vector<models::dto::DeliveryResponse>
  PostgresStorageComponent::GetDeliveriesBySender(int64_t sender_id, int from, int to)
  {
    const std::string query = R"(
    SELECT d.id, d.parcel_id, d.sender_id, d.receiver_id, d.tracking_number, 
           d.status, d.from_address, d.to_address, d.created_at, d.delivered_at,
           p.weight, p.dimensions, p.declared_value, p.description
    FROM deliveries d
    JOIN parcels p ON d.parcel_id = p.id
    WHERE d.sender_id = $1
    ORDER BY d.created_at DESC
    LIMIT $2
    OFFSET $3
  )";

    auto rows = cluster_->Execute(ClusterHostType::kMaster, query, sender_id,
                                  to - from + 1, from - 1);

    std::vector<models::dto::DeliveryResponse> result;
    result.reserve(rows.Size());

    for (const auto &row : rows)
    {
      result.push_back(
          {row["id"].As<int64_t>(), row["parcel_id"].As<int64_t>(),
           row["sender_id"].As<int64_t>(), row["receiver_id"].As<int64_t>(),
           row["tracking_number"].As<std::string>(), row["status"].As<std::string>(),
           row["from_address"].As<std::string>(), row["to_address"].As<std::string>(),
           row["created_at"].As<userver::storages::postgres::TimePointTz>(),
           row["delivered_at"].As<std::optional<userver::storages::postgres::TimePointTz>>()});
    }
    return result;
  }

  std::vector<models::dto::DeliveryResponse>
  PostgresStorageComponent::GetDeliveriesByReceiver(int64_t receiver_id, int from, int to)
  {
    const std::string query = R"(
    SELECT d.id, d.parcel_id, d.sender_id, d.receiver_id, d.tracking_number, 
           d.status, d.from_address, d.to_address, d.created_at, d.delivered_at,
           p.weight, p.dimensions, p.declared_value, p.description
    FROM deliveries d
    JOIN parcels p ON d.parcel_id = p.id
    WHERE d.receiver_id = $1
    ORDER BY d.created_at DESC
    LIMIT $2
    OFFSET $3
  )";

    auto rows = cluster_->Execute(ClusterHostType::kMaster, query, receiver_id,
                                  to - from + 1, from - 1);

    std::vector<models::dto::DeliveryResponse> result;
    result.reserve(rows.Size());

    for (const auto &row : rows)
    {
      result.push_back(
          {row["id"].As<int64_t>(), row["parcel_id"].As<int64_t>(),
           row["sender_id"].As<int64_t>(), row["receiver_id"].As<int64_t>(),
           row["tracking_number"].As<std::string>(), row["status"].As<std::string>(),
           row["from_address"].As<std::string>(), row["to_address"].As<std::string>(),
           row["created_at"].As<userver::storages::postgres::TimePointTz>(),
           row["delivered_at"].As<std::optional<userver::storages::postgres::TimePointTz>>()});
    }
    return result;
  }

  std::optional<models::dto::DeliveryResponse>
  PostgresStorageComponent::GetDeliveryByTrackingNumber(const std::string &tracking_number)
  {
    const std::string query = R"(
    SELECT id, parcel_id, sender_id, receiver_id, tracking_number, 
           status, from_address, to_address, created_at, delivered_at 
    FROM deliveries 
    WHERE tracking_number = $1
  )";

    auto row_opt = cluster_->Execute(ClusterHostType::kMaster, query, tracking_number)
                       .AsOptionalSingleRow<models::dto::DeliveryResponse>(
                           userver::storages::postgres::kRowTag);

    if (!row_opt)
    {
      return std::nullopt;
    }

    return row_opt.value();
  }

  bool PostgresStorageComponent::UpdateDeliveryStatus(int64_t id, const std::string &status)
  {
    std::string delivered_at_clause = "";
    if (status == "delivered")
    {
      delivered_at_clause = ", delivered_at = NOW()";
    }

    const std::string query = "UPDATE deliveries SET status = $1" + delivered_at_clause + " WHERE id = $2";
    auto result = cluster_->Execute(ClusterHostType::kMaster, query, status, id);
    return result.RowsAffected() > 0;
  }

} // namespace components