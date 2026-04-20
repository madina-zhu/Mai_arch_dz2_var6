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
    inline static constexpr const char *uniqueViolation = "uniqueViolation";
    inline static constexpr const char *constraintViolation = "constraintViolation";
    inline static constexpr const char *dataViolation = "dataViolation";

    static constexpr std::string_view kName = "mongo-storage-component";

    explicit MongoStorageComponent(
        const userver::components::ComponentConfig &config,
        const userver::components::ComponentContext &context);

    // ==================== PARCEL OPERATIONS (MongoDB) ====================
    std::string CreateParcel(const models::dto::ParcelCreateRequest &request);
    std::optional<models::dto::ParcelResponse>
    GetParcelById(const std::string &parcel_id);
    std::vector<models::dto::ParcelResponse>
    GetParcelsByFromCity(const std::string &city, int to, int from);
    std::vector<models::dto::ParcelResponse>
    GetParcelsByWeightRange(double min_weight, double max_weight, int to, int from);
    std::vector<models::dto::ParcelResponse>
    GetParcelsBySenderId(int64_t sender_id);
    bool UpdateParcelStatus(const std::string &parcel_id,
                            const std::string &status);
    bool AddFeatureToParcel(const std::string &parcel_id,
                            const std::string &feature);
    bool DeleteParcel(const std::string &parcel_id);

    // ==================== DELIVERY OPERATIONS (MongoDB) ====================
    std::string CreateDelivery(const models::dto::DeliveryCreateRequest &request);
    std::vector<models::dto::DeliveryResponse>
    GetDeliveriesByParcelId(const std::string &parcel_id);
    std::optional<models::dto::DeliveryResponse>
    GetDeliveryById(const std::string &delivery_id);
    bool AddTrackingEventToDelivery(const std::string &delivery_id,
                                    const std::string &status,
                                    const std::string &location,
                                    const std::string &reason);
    bool CancelDelivery(const std::string &delivery_id, const std::string &reason);

  private:
    userver::storages::mongo::PoolPtr mongo_pool_;

    userver::storages::mongo::Collection parcels_collection_;
    userver::storages::mongo::Collection deliveries_collection_;

    userver::formats::bson::Value
    MapParcelToBson(const models::dto::ParcelCreateRequest &req);

    models::dto::ParcelResponse
    MapBsonToParcel(const userver::formats::bson::Value &doc);

    userver::formats::bson::Value
    MapDeliveryToBson(const models::dto::DeliveryCreateRequest &req);

    models::dto::DeliveryResponse
    MapBsonToDelivery(const userver::formats::bson::Value &doc);
  };

} // namespace components