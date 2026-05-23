#pragma once

#include <optional>
#include <string>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <vector>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>

#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>

#include <userver/formats/bson.hpp>
#include <userver/formats/bson/value.hpp>

#include <userver/utils/datetime.hpp>

#include "../models/dto.hpp"

namespace components
{

  class MongoStorageComponent : public userver::components::ComponentBase
  {
  public:
    inline static constexpr const char *UNIQUE_VIOLATION = "uniqueViolation";
    inline static constexpr const char *CONSTRAINT_VIOLATION = "constraintViolation";
    inline static constexpr const char *DATA_VIOLATION = "dataViolation";

    static constexpr std::string_view kName = "mongo-storage-component";

    explicit MongoStorageComponent(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context);

    // ==================== PARCEL OPERATIONS ====================
    std::string CreateParcel(const models::dto::ParcelCreateRequest &request);
    std::optional<models::dto::ParcelResponse>
    GetParcelById(const std::string &parcel_id);
    std::vector<models::dto::ParcelResponse>
    GetParcelsBySender(int64_t sender_id, int to, int from);
    std::vector<models::dto::ParcelResponse>
    GetParcelsByStatus(const std::string &status, int to, int from);
    bool UpdateParcelStatus(const std::string &parcel_id,
                            const std::string &status);
    bool AddFeatureToParcel(const std::string &parcel_id,
                            const std::string &feature);
    bool DeleteParcel(const std::string &parcel_id);

    // ==================== DELIVERY OPERATIONS ====================
    std::string CreateDelivery(const models::dto::DeliveryCreateRequest &request);
    std::optional<models::dto::DeliveryResponse>
    GetDeliveryById(const std::string &delivery_id);
    std::optional<models::dto::DeliveryResponse>
    GetDeliveryByTrackingNumber(const std::string &tracking_number);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesBySender(int64_t sender_id, int to, int from);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesByReceiver(int64_t receiver_id, int to, int from);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesByStatus(const std::string &status, int to, int from);
    bool UpdateDeliveryStatus(const std::string &delivery_id,
                              const std::string &status,
                              const std::string &location,
                              const std::string &comment);
    bool AddDeliveryEvent(const std::string &delivery_id,
                          const std::string &status,
                          const std::string &location,
                          const std::string &comment);

  private:
    userver::storages::mongo::PoolPtr mongo_pool_;

    userver::storages::mongo::Collection parcels_collection_;
    userver::storages::mongo::Collection deliveries_collection_;

    // Mapping functions
    userver::formats::bson::Value
    MapParcelToBson(const models::dto::ParcelCreateRequest &req);

    models::dto::ParcelResponse
    MapBsonToParcel(const userver::formats::bson::Value &doc);

    userver::formats::bson::Value
    MapDeliveryToBson(const models::dto::DeliveryCreateRequest &req);

    models::dto::DeliveryResponse
    MapBsonToDelivery(const userver::formats::bson::Value &doc);

    // Helpers
    std::string GenerateTrackingNumber();
    std::string GenerateUuid();
  };

} // namespace components