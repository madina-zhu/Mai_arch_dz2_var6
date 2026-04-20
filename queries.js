// Delivery Service API — MongoDB Queries
// Запуск: docker exec -i mongodb_delivery mongosh -u admin -p password
// Затем: load("./queries.js");

db = db.getSiblingDB("mongo_delivery_db");

print("=== Delivery Service MongoDB Queries ===");
print("");

// ============================================================================
// PARCELS OPERATIONS (Посылки)
// ============================================================================

/**
 * API: Создание новой посылки
 * POST /parcels
 */
function createParcel(senderId, recipientId, type, title, fromCity, toCity, address, weight, price, details, features, status) {
    const parcel = {
        _id: crypto.randomUUID(),
        sender_id: senderId,
        recipient_id: recipientId,
        type: type,
        title: title,
        from_city: fromCity,
        to_city: toCity,
        address: address,
        weight: weight,
        price: price,
        details: details,
        features: features || [],
        status: status || "pending",
        created_at: new Date()
    };

    const result = db.parcels.insertOne(parcel);
    print("[Mongo] Parcel created with ID:");
    printjson(result.insertedId);
    return result.insertedId;
}

/**
 * API: Получение посылок пользователя (по отправителю)
 * GET /parcels?sender_id=1
 */
function findParcelsBySender(senderId) {
    return db.parcels.find({ sender_id: senderId }).toArray();
}

/**
 * API: Получение посылок пользователя (по получателю)
 * GET /parcels?recipient_id=2
 */
function findParcelsByRecipient(recipientId) {
    return db.parcels.find({ recipient_id: recipientId }).toArray();
}

/**
 * API: Поиск посылок по городу отправления
 * GET /parcels?from_city=Moscow
 */
function findParcelsByFromCity(city) {
    return db.parcels.find({ from_city: city }).toArray();
}

/**
 * API: Поиск посылок по городу назначения
 * GET /parcels?to_city=Saint+Petersburg
 */
function findParcelsByToCity(city) {
    return db.parcels.find({ to_city: city }).toArray();
}

/**
 * API: Поиск посылок по диапазону веса
 * GET /parcels?min_weight=1&max_weight=10
 */
function findParcelsByWeightRange(minWeight, maxWeight) {
    return db.parcels.find({
        weight: { $gte: minWeight, $lte: maxWeight }
    }).toArray();
}

/**
 * API: Поиск посылок по типу (с использованием $in)
 * GET /parcels?type=express&type=fragile
 */
function findParcelsByTypes(typesArray) {
    return db.parcels.find({
        type: { $in: typesArray }
    }).toArray();
}

/**
 * API: Добавление особенности к посылке (оператор $addToSet)
 * PATCH /parcels/{id}/features
 */
function addFeatureToParcel(parcelId, feature) {
    const result = db.parcels.updateOne(
        { _id: parcelId },
        { $addToSet: { features: feature } }
    );
    return result.modifiedCount > 0;
}

/**
 * API: Удаление особенности из посылки (оператор $pull)
 * DELETE /parcels/{id}/features/{feature}
 */
function removeFeatureFromParcel(parcelId, feature) {
    const result = db.parcels.updateOne(
        { _id: parcelId },
        { $pull: { features: feature } }
    );
    return result.modifiedCount > 0;
}

/**
 * API: Изменение статуса посылки ($set)
 * PATCH /parcels/{id}/status
 */
function updateParcelStatus(parcelId, newStatus) {
    const result = db.parcels.updateOne(
        { _id: parcelId },
        { $set: { status: newStatus, updated_at: new Date() } }
    );
    return result.modifiedCount > 0;
}

/**
 * API: Удаление посылки (только если статус "pending")
 * DELETE /parcels/{id}
 */
function deleteParcelIfPending(parcelId) {
    const result = db.parcels.deleteOne({
        _id: parcelId,
        status: "pending"
    });
    return result.deletedCount > 0;
}

// ============================================================================
// DELIVERIES OPERATIONS (Доставки)
// ============================================================================

/**
 * API: Создание доставки
 * POST /deliveries
 */
function createDelivery(parcelId, courierId, pickupTime, estimatedDelivery) {
    const delivery = {
        _id: crypto.randomUUID(),
        parcel_id: parcelId,
        courier_id: courierId,
        pickup_time: new Date(pickupTime),
        estimated_delivery: new Date(estimatedDelivery),
        actual_delivery: null,
        status: "pending",
        tracking_history: [
            { status: "created", timestamp: new Date(), location: "system" }
        ],
        created_at: new Date()
    };

    const result = db.deliveries.insertOne(delivery);
    print("[Mongo] Delivery created with ID:");
    printjson(result.insertedId);
    return result.insertedId;
}

/**
 * API: Получение доставок по получателю (через JOIN с parcels)
 * GET /deliveries?recipient_id=2
 */
function findDeliveriesByRecipient(recipientId) {
    return db.deliveries.aggregate([
        {
            $lookup: {
                from: "parcels",
                localField: "parcel_id",
                foreignField: "_id",
                as: "parcel_info"
            }
        },
        { $unwind: "$parcel_info" },
        { $match: { "parcel_info.recipient_id": recipientId } },
        {
            $project: {
                _id: 1,
                parcel_id: 1,
                courier_id: 1,
                pickup_time: 1,
                estimated_delivery: 1,
                actual_delivery: 1,
                status: 1,
                tracking_history: 1,
                created_at: 1,
                parcel_title: "$parcel_info.title",
                from_city: "$parcel_info.from_city",
                to_city: "$parcel_info.to_city"
            }
        }
    ]).toArray();
}

/**
 * API: Получение доставок по отправителю (через JOIN с parcels)
 * GET /deliveries?sender_id=1
 */
function findDeliveriesBySender(senderId) {
    return db.deliveries.aggregate([
        {
            $lookup: {
                from: "parcels",
                localField: "parcel_id",
                foreignField: "_id",
                as: "parcel_info"
            }
        },
        { $unwind: "$parcel_info" },
        { $match: { "parcel_info.sender_id": senderId } },
        {
            $project: {
                _id: 1,
                parcel_id: 1,
                courier_id: 1,
                pickup_time: 1,
                estimated_delivery: 1,
                actual_delivery: 1,
                status: 1,
                tracking_history: 1,
                created_at: 1,
                parcel_title: "$parcel_info.title",
                from_city: "$parcel_info.from_city",
                to_city: "$parcel_info.to_city"
            }
        }
    ]).toArray();
}

/**
 * API: Добавление события в историю доставки ($push)
 * POST /deliveries/{id}/track
 */
function addTrackingEvent(deliveryId, status, location, optionalReason) {
    const event = {
        status: status,
        timestamp: new Date(),
        location: location
    };
    if (optionalReason) {
        event.reason = optionalReason;
    }

    const updateDoc = {
        $push: { tracking_history: event },
        $set: { status: status }
    };

    if (status === "delivered") {
        updateDoc.$set.actual_delivery = new Date();
    }

    const result = db.deliveries.updateOne(
        { _id: deliveryId },
        updateDoc
    );
    return result.modifiedCount > 0;
}

/**
 * API: Отмена доставки (с указанием причины)
 * PATCH /deliveries/{id}/cancel
 */
function cancelDelivery(deliveryId, reason) {
    const result = db.deliveries.updateOne(
        { _id: deliveryId, status: { $nin: ["delivered", "cancelled"] } },
        {
            $set: { status: "cancelled" },
            $push: {
                tracking_history: {
                    status: "cancelled",
                    timestamp: new Date(),
                    location: "system",
                    reason: reason
                }
            }
        }
    );
    return result.modifiedCount > 0;
}

/**
 * API: Получение полной истории доставки
 * GET /deliveries/{id}/tracking
 */
function getTrackingHistory(deliveryId) {
    const delivery = db.deliveries.findOne(
        { _id: deliveryId },
        { projection: { tracking_history: 1, status: 1, estimated_delivery: 1 } }
    );
    return delivery;
}

// ============================================================================
// EXAMPLE USAGE (раскомментируйте для тестирования)
// ============================================================================

// print("\n=== Примеры запросов ===");
// print("\n1. Посылки отправителя #1:");
// printjson(findParcelsBySender(1));
//
// print("\n2. Посылки из Москвы:");
// printjson(findParcelsByFromCity("Moscow"));
//
// print("\n3. Посылки весом от 1 до 10 кг:");
// printjson(findParcelsByWeightRange(1, 10));
//
// print("\n4. Доставки для получателя #2:");
// printjson(findDeliveriesByRecipient(2));
//
// print("\n5. Доставки для отправителя #1:");
// printjson(findDeliveriesBySender(1));

print("\n=== Queries loaded successfully ===");