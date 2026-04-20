#include "mongo_storage_component.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <userver/formats/bson.hpp>
#include <userver/formats/bson/document.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/iterator.hpp>
#include <userver/formats/bson/serialize.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/bson/value.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/formats/common/conversion_stack.hpp>
#include <userver/formats/common/utils.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

bool IsDuplicateKeyError(const std::exception &e)
{
  const std::string msg = e.what();
  return msg.find("E11000") != std::string::npos ||
         msg.find("duplicate key") != std::string::npos;
}

auto handleMongoError = [](const std::string &msg, const std::string &code)
{
  LOG_WARNING() << msg;
  return code;
};

namespace components
{

  // ==================== CONSTRUCTOR ====================

  MongoStorageComponent::MongoStorageComponent(
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &context)
      : ComponentBase(config, context),
        mongo_pool_(
            context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
        parcels_collection_(mongo_pool_->GetCollection("parcels")),
        deliveries_collection_(mongo_pool_->GetCollection("deliveries")) {}

  // ==================== PARCEL OPERATIONS ====================

  std::string MongoStorageComponent::CreateParcel(
      const models::dto::ParcelCreateRequest &request)
  {

    try
    {
      auto doc = MapParcelToBson(request);
      parcels_collection_.InsertOne(doc);
      return doc["_id"].As<std::string>();
    }
    catch (const std::exception &e)
    {
      if (IsDuplicateKeyError(e))
      {
        return handleMongoError("Failed to create parcel: duplicate key violation",
                                uniqueViolation);
      }

      return handleMongoError(
          "Failed to create parcel: data constraints were not matched.",
          constraintViolation);
    }
  }

  std::optional<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelById(const std::string &id)
  {

    auto doc = parcels_collection_.FindOne(userver::formats::bson::MakeDoc(
        "_id", id));

    if (!doc)
      return std::nullopt;

    return MapBsonToParcel(*doc);
  }

  std::vector<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelsByFromCity(const std::string &city, int to, int from)
  {

    std::vector<models::dto::ParcelResponse> result;

    auto cursor = parcels_collection_.Find(
        userver::formats::bson::MakeDoc("from_city", city),
        userver::storages::mongo::options::Limit(to - from + 1),
        userver::storages::mongo::options::Skip(from - 1));

    result.reserve(to - from + 1);

    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToParcel(doc));
    }

    return result;
  }

  std::vector<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelsByWeightRange(double min_weight, double max_weight, int to, int from)
  {

    std::vector<models::dto::ParcelResponse> result;

    auto cursor = parcels_collection_.Find(
        userver::formats::bson::MakeDoc(
            "weight", userver::formats::bson::MakeDoc("$gte", min_weight, "$lte", max_weight)),
        userver::storages::mongo::options::Limit(to - from + 1),
        userver::storages::mongo::options::Skip(from - 1));

    result.reserve(to - from + 1);

    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToParcel(doc));
    }

    return result;
  }

  std::vector<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelsBySenderId(int64_t sender_id)
  {

    std::vector<models::dto::ParcelResponse> result;

    auto cursor = parcels_collection_.Find(
        userver::formats::bson::MakeDoc("sender_id", sender_id));

    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToParcel(doc));
    }

    return result;
  }

  bool MongoStorageComponent::UpdateParcelStatus(const std::string &parcel_id,
                                                 const std::string &status)
  {
    try
    {
      auto result = parcels_collection_.UpdateOne(
          userver::formats::bson::MakeDoc("_id", parcel_id),
          userver::formats::bson::MakeDoc(
              "$set", userver::formats::bson::MakeDoc("status", status)));

      return result.MatchedCount() > 0;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "UpdateParcelStatus failed: " << e.what();
      return false;
    }
  }

  bool MongoStorageComponent::AddFeatureToParcel(const std::string &parcel_id,
                                                 const std::string &feature)
  {
    try
    {
      auto result = parcels_collection_.UpdateOne(
          userver::formats::bson::MakeDoc("_id", parcel_id),
          userver::formats::bson::MakeDoc(
              "$addToSet", userver::formats::bson::MakeDoc("features", feature)));

      return result.MatchedCount() > 0;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "AddFeatureToParcel failed: " << e.what();
      return false;
    }
  }

  bool MongoStorageComponent::DeleteParcel(const std::string &parcel_id)
  {

    try
    {
      auto result = parcels_collection_.DeleteOne(
          userver::formats::bson::MakeDoc("_id", parcel_id));

      return result.DeletedCount() > 0;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "DeleteParcel failed: " << e.what();
      return false;
    }
  }

  // ==================== DELIVERY OPERATIONS ====================

  std::string MongoStorageComponent::CreateDelivery(
      const models::dto::DeliveryCreateRequest &request)
  {

    try
    {
      auto parcel = parcels_collection_.FindOne(
          userver::formats::bson::MakeDoc("_id", request.parcel_id));

      if (!parcel)
      {
        return handleMongoError("Parcel not found", dataViolation);
      }

      auto doc = MapDeliveryToBson(request);
      deliveries_collection_.InsertOne(doc);
      return doc["_id"].As<std::string>();
    }
    catch (const std::exception &e)
    {

      if (IsDuplicateKeyError(e))
      {
        return handleMongoError("Failed to create delivery: duplicate key violation",
                                uniqueViolation);
      }

      return handleMongoError(
          "Failed to create delivery: data constraints were not matched.",
          constraintViolation);
    }
  }

  std::vector<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveriesByParcelId(const std::string &parcel_id)
  {

    std::vector<models::dto::DeliveryResponse> result;

    auto cursor = deliveries_collection_.Find(userver::formats::bson::MakeDoc(
        "parcel_id", parcel_id));

    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToDelivery(doc));
    }

    return result;
  }

  std::optional<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveryById(const std::string &delivery_id)
  {

    auto doc = deliveries_collection_.FindOne(
        userver::formats::bson::MakeDoc("_id", delivery_id));

    if (!doc)
      return std::nullopt;

    return MapBsonToDelivery(*doc);
  }

  bool MongoStorageComponent::AddTrackingEventToDelivery(const std::string &delivery_id,
                                                         const std::string &status,
                                                         const std::string &location,
                                                         const std::string &reason)
  {

    userver::formats::bson::ValueBuilder event_builder;
    event_builder["status"] = status;
    event_builder["timestamp"] = userver::utils::datetime::Now();
    event_builder["location"] = location;
    if (!reason.empty())
    {
      event_builder["reason"] = reason;
    }

    userver::formats::bson::ValueBuilder update_builder;
    update_builder["$push"]["tracking_history"] = event_builder.ExtractValue();
    update_builder["$set"]["status"] = status;

    if (status == "delivered")
    {
      update_builder["$set"]["actual_delivery"] = userver::utils::datetime::Now();
    }

    auto result = deliveries_collection_.UpdateOne(
        userver::formats::bson::MakeDoc("_id", delivery_id),
        update_builder.ExtractValue());

    return result.MatchedCount() > 0;
  }

  bool MongoStorageComponent::CancelDelivery(const std::string &delivery_id,
                                             const std::string &reason)
  {

    auto result = deliveries_collection_.UpdateOne(
        userver::formats::bson::MakeDoc("_id", delivery_id),
        userver::formats::bson::MakeDoc(
            "$set", userver::formats::bson::MakeDoc("status", "cancelled"),
            "$push", userver::formats::bson::MakeDoc("tracking_history", userver::formats::bson::MakeDoc("status", "cancelled", "timestamp", userver::utils::datetime::Now(), "location", "system", "reason", reason))));

    return result.MatchedCount() > 0;
  }

  // ==================== MAPPING ====================

  userver::formats::bson::Value MongoStorageComponent::MapParcelToBson(
      const models::dto::ParcelCreateRequest &req)
  {

    const auto id = userver::utils::generators::GenerateUuid();

    return userver::formats::bson::MakeDoc(
        "_id", id,
        "sender_id", req.sender_id,
        "recipient_id", req.recipient_id,
        "type", req.type,
        "title", req.title,
        "from_city", req.from_city,
        "to_city", req.to_city,
        "address", req.address.As<userver::formats::bson::Value>(),
        "weight", req.weight,
        "price", req.price,
        "details", req.details.As<userver::formats::bson::Value>(),
        "features", req.features.As<userver::formats::bson::Value>(),
        "status", req.status,
        "created_at", std::chrono::system_clock::now());
  }

  models::dto::ParcelResponse MongoStorageComponent::MapBsonToParcel(
      const userver::formats::bson::Value &doc)
  {

    models::dto::ParcelResponse res;

    res.id = doc["_id"].As<std::string>();
    res.sender_id = doc["sender_id"].As<int64_t>();
    res.recipient_id = doc["recipient_id"].As<int64_t>();
    res.type = doc["type"].As<std::string>();
    res.title = doc["title"].As<std::string>();
    res.from_city = doc["from_city"].As<std::string>();
    res.to_city = doc["to_city"].As<std::string>();
    res.weight = doc["weight"].As<double>();
    res.price = doc["price"].As<double>();
    res.status = doc["status"].As<std::string>();

    res.address = doc["address"].As<userver::formats::json::Value>();
    res.details = doc["details"].As<userver::formats::json::Value>();
    res.features = doc["features"].As<userver::formats::json::Value>();

    res.created_at = doc["created_at"].As<std::chrono::system_clock::time_point>();

    return res;
  }

  userver::formats::bson::Value MongoStorageComponent::MapDeliveryToBson(
      const models::dto::DeliveryCreateRequest &req)
  {

    const auto id = userver::utils::generators::GenerateUuid();

    auto tracking_history = userver::formats::bson::MakeDoc(
        "status", "created",
        "timestamp", userver::utils::datetime::Now(),
        "location", "system");

    return userver::formats::bson::MakeDoc(
        "_id", id,
        "parcel_id", req.parcel_id,
        "recipient_id", req.recipient_id,
        "courier_id", req.courier_id,
        "pickup_time", req.pickup_time,
        "estimated_delivery", req.estimated_delivery,
        "actual_delivery", userver::formats::bson::kNull,
        "status", "pending",
        "tracking_history", userver::formats::bson::MakeArray(tracking_history),
        "created_at", std::chrono::system_clock::now());
  }

  models::dto::DeliveryResponse MongoStorageComponent::MapBsonToDelivery(
      const userver::formats::bson::Value &doc)
  {

    models::dto::DeliveryResponse res;

    res.id = doc["_id"].As<std::string>();
    res.parcel_id = doc["parcel_id"].As<std::string>();
    res.recipient_id = doc["recipient_id"].As<int64_t>();
    res.courier_id = doc["courier_id"].As<int64_t>();
    res.pickup_time = doc["pickup_time"].As<std::chrono::system_clock::time_point>();
    res.estimated_delivery = doc["estimated_delivery"].As<std::chrono::system_clock::time_point>();

    if (!doc["actual_delivery"].IsNull())
    {
      res.actual_delivery = doc["actual_delivery"].As<std::chrono::system_clock::time_point>();
    }

    res.status = doc["status"].As<std::string>();

    // Парсим tracking_history
    for (const auto &event_bson : doc["tracking_history"])
    {
      models::dto::TrackingEvent event;
      event.status = event_bson["status"].As<std::string>();
      event.timestamp = event_bson["timestamp"].As<std::chrono::system_clock::time_point>();
      event.location = event_bson["location"].As<std::string>();
      if (event_bson.HasMember("reason"))
      {
        event.reason = event_bson["reason"].As<std::string>();
      }
      res.tracking_history.push_back(event);
    }

    res.created_at = doc["created_at"].As<std::chrono::system_clock::time_point>();

    return res;
  }

} // namespace components