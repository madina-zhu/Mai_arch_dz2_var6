#include "mongo_storage_component.hpp"

#include <chrono>
#include <cstdint>
#include <random>
#include <string>

#include <userver/formats/bson.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

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

  // ==================== HELPERS ====================

  std::string MongoStorageComponent::GenerateUuid()
  {
    return userver::utils::generators::GenerateUuid();
  }

  std::string MongoStorageComponent::GenerateTrackingNumber()
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

    static uint64_t counter = 1;
    return "DEL-" + std::to_string(counter++) + "-" + suffix;
  }

  // ==================== PARCEL OPERATIONS ====================

  std::string MongoStorageComponent::CreateParcel(
      const models::dto::ParcelCreateRequest &request)
  {
    try
    {
      auto doc = MapParcelToBson(request);
      parcels_collection_.InsertOne(doc);
      LOG_INFO() << "Created parcel with id: " << doc["_id"].As<std::string>();
      return doc["_id"].As<std::string>();
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to create parcel: " << e.what();
      return CONSTRAINT_VIOLATION;
    }
  }

  std::optional<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelById(const std::string &id)
  {
    auto doc = parcels_collection_.FindOne(
        userver::formats::bson::MakeDoc("_id", id));
    if (!doc)
    {
      return std::nullopt;
    }
    return MapBsonToParcel(*doc);
  }

  std::vector<models::dto::ParcelResponse>
  MongoStorageComponent::GetParcelsBySender(int64_t sender_id, int to, int from)
  {
    std::vector<models::dto::ParcelResponse> result;
    auto cursor = parcels_collection_.Find(
        userver::formats::bson::MakeDoc("sender_id", sender_id),
        userver::storages::mongo::options::Limit(to - from + 1),
        userver::storages::mongo::options::Skip(from - 1));
    result.reserve(to - from + 1);
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
        return DATA_VIOLATION;
      }

      auto doc = MapDeliveryToBson(request);
      deliveries_collection_.InsertOne(doc);

      UpdateParcelStatus(request.parcel_id, "assigned");

      LOG_INFO() << "Created delivery with id: " << doc["_id"].As<std::string>()
                 << ", tracking: " << doc["tracking_number"].As<std::string>();
      return doc["_id"].As<std::string>();
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "Failed to create delivery: " << e.what();
      return CONSTRAINT_VIOLATION;
    }
  }

  std::optional<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveryById(const std::string &delivery_id)
  {
    auto doc = deliveries_collection_.FindOne(
        userver::formats::bson::MakeDoc("_id", delivery_id));
    if (!doc)
    {
      return std::nullopt;
    }
    return MapBsonToDelivery(*doc);
  }

  std::optional<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveryByTrackingNumber(const std::string &tracking_number)
  {
    auto doc = deliveries_collection_.FindOne(
        userver::formats::bson::MakeDoc("tracking_number", tracking_number));
    if (!doc)
    {
      return std::nullopt;
    }
    return MapBsonToDelivery(*doc);
  }

  std::vector<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveriesBySender(int64_t sender_id, int to, int from)
  {
    std::vector<models::dto::DeliveryResponse> result;
    auto cursor = deliveries_collection_.Find(
        userver::formats::bson::MakeDoc("sender_id", sender_id),
        userver::storages::mongo::options::Limit(to - from + 1),
        userver::storages::mongo::options::Skip(from - 1));
    result.reserve(to - from + 1);
    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToDelivery(doc));
    }
    return result;
  }

  std::vector<models::dto::DeliveryResponse>
  MongoStorageComponent::GetDeliveriesByReceiver(int64_t receiver_id, int to, int from)
  {
    std::vector<models::dto::DeliveryResponse> result;
    auto cursor = deliveries_collection_.Find(
        userver::formats::bson::MakeDoc("receiver_id", receiver_id),
        userver::storages::mongo::options::Limit(to - from + 1),
        userver::storages::mongo::options::Skip(from - 1));
    result.reserve(to - from + 1);
    for (const auto &doc : cursor)
    {
      result.push_back(MapBsonToDelivery(doc));
    }
    return result;
  }

  bool MongoStorageComponent::UpdateDeliveryStatus(const std::string &delivery_id,
                                                   const std::string &status,
                                                   const std::string &location,
                                                   const std::string &comment)
  {
    try
    {
      auto update_doc = userver::formats::bson::MakeDoc(
          "$set", userver::formats::bson::MakeDoc("status", status),
          "$push", userver::formats::bson::MakeDoc("events", userver::formats::bson::MakeDoc("status", status, "timestamp", userver::utils::datetime::Now(), "location", location, "comment", comment)));

      if (status == "delivered")
      {
        update_doc = userver::formats::bson::MakeDoc(
            "$set", userver::formats::bson::MakeDoc("status", status, "delivered_at", userver::utils::datetime::Now()),
            "$push", userver::formats::bson::MakeDoc("events", userver::formats::bson::MakeDoc("status", status, "timestamp", userver::utils::datetime::Now(), "location", location, "comment", comment)));
      }

      auto result = deliveries_collection_.UpdateOne(
          userver::formats::bson::MakeDoc("_id", delivery_id),
          update_doc);

      if (result.MatchedCount() > 0 && (status == "delivered" || status == "cancelled"))
      {
        auto delivery = GetDeliveryById(delivery_id);
        if (delivery)
        {
          UpdateParcelStatus(delivery->parcel_id,
                             status == "delivered" ? "delivered" : "created");
        }
      }

      return result.MatchedCount() > 0;
    }
    catch (const std::exception &e)
    {
      LOG_ERROR() << "UpdateDeliveryStatus failed: " << e.what();
      return false;
    }
  }

  // ==================== MAPPING FUNCTIONS ====================

  userver::formats::bson::Value MongoStorageComponent::MapParcelToBson(
      const models::dto::ParcelCreateRequest &req)
  {
    const auto id = GenerateUuid();

    return userver::formats::bson::MakeDoc(
        "_id", id,
        "sender_id", req.sender_id,
        "weight", req.weight,
        "dimensions", req.dimensions,
        "declared_value", req.declared_value,
        "description", req.description.value_or(""),
        "status", "created",
        "created_at", std::chrono::system_clock::now());
  }

  models::dto::ParcelResponse MongoStorageComponent::MapBsonToParcel(
      const userver::formats::bson::Value &doc)
  {
    models::dto::ParcelResponse res;
    res.id = doc["_id"].As<std::string>();
    res.sender_id = doc["sender_id"].As<int64_t>();
    res.weight = doc["weight"].As<double>();
    res.dimensions = doc["dimensions"].As<std::string>();
    res.declared_value = doc["declared_value"].As<double>();
    if (doc.HasMember("description") && !doc["description"].As<std::string>().empty())
    {
      res.description = doc["description"].As<std::string>();
    }
    res.status = doc["status"].As<std::string>();
    res.created_at = doc["created_at"].As<std::chrono::system_clock::time_point>();
    return res;
  }

  userver::formats::bson::Value MongoStorageComponent::MapDeliveryToBson(
      const models::dto::DeliveryCreateRequest &req)
  {
    const auto id = GenerateUuid();
    const auto tracking_number = GenerateTrackingNumber();

    auto events_array = userver::formats::bson::ValueBuilder{};
    events_array.PushBack(userver::formats::bson::MakeDoc(
        "status", "pending",
        "timestamp", std::chrono::system_clock::now(),
        "location", req.from_address.substr(0, req.from_address.find(','))));

    return userver::formats::bson::MakeDoc(
        "_id", id,
        "parcel_id", req.parcel_id,
        "sender_id", req.sender_id,
        "receiver_id", req.receiver_id,
        "tracking_number", tracking_number,
        "status", "pending",
        "from_address", req.from_address,
        "to_address", req.to_address,
        "created_at", std::chrono::system_clock::now(),
        "delivered_at", nullptr,
        "events", events_array.ExtractValue());
  }

  models::dto::DeliveryResponse MongoStorageComponent::MapBsonToDelivery(
      const userver::formats::bson::Value &doc)
  {
    models::dto::DeliveryResponse res;
    res.id = doc["_id"].As<std::string>();
    res.parcel_id = doc["parcel_id"].As<std::string>();
    res.sender_id = doc["sender_id"].As<int64_t>();
    res.receiver_id = doc["receiver_id"].As<int64_t>();
    res.tracking_number = doc["tracking_number"].As<std::string>();
    res.status = doc["status"].As<std::string>();
    res.from_address = doc["from_address"].As<std::string>();
    res.to_address = doc["to_address"].As<std::string>();
    res.created_at = doc["created_at"].As<std::chrono::system_clock::time_point>();

    if (doc.HasMember("delivered_at") && !doc["delivered_at"].IsNull())
    {
      res.delivered_at = doc["delivered_at"].As<std::chrono::system_clock::time_point>();
    }

    if (doc.HasMember("events"))
    {
      for (const auto &event_bson : doc["events"])
      {
        models::dto::DeliveryEvent event;
        event.status = event_bson["status"].As<std::string>();
        event.timestamp = event_bson["timestamp"].As<std::chrono::system_clock::time_point>();
        event.location = event_bson["location"].As<std::string>();
        if (event_bson.HasMember("comment"))
        {
          event.comment = event_bson["comment"].As<std::string>();
        }
        res.events.push_back(event);
      }
    }

    return res;
  }

} // namespace components