// ============================================================================
// MongoDB Queries for Delivery API (вариант 6: Доставка)
// ============================================================================

db = db.getSiblingDB("delivery_db");

print("=== MongoDB Queries Ready ===");
print("Available functions:");
print("  - createParcel(senderId, weight, dimensions, declaredValue, description)");
print("  - findParcelsBySender(senderId)");
print("  - findParcelsByStatus(status)");
print("  - createDelivery(parcelId, senderId, receiverId, fromAddress, toAddress)");
print("  - findDeliveryByTrackingNumber(trackingNumber)");
print("  - findDeliveriesBySender(senderId)");
print("  - findDeliveriesByReceiver(receiverId)");
print("  - updateDeliveryStatus(deliveryId, newStatus, location, comment)");
print("  - addDeliveryEvent(deliveryId, status, location, comment)");
print("  - getTrackingHistory(trackingNumber)");
print("  - findDeliveriesByStatus(status)");
print("  - addFeatureToParcel(parcelId, feature)");

// ============================================================================
// PARCEL OPERATIONS
// ============================================================================

function createParcel(senderId, weight, dimensions, declaredValue, description) {
    const parcel = {
        _id: uuid(),
        sender_id: senderId,
        weight: weight,
        dimensions: dimensions,
        declared_value: declaredValue,
        description: description || null,
        status: "created",
        created_at: new Date(),
        features: []
    };
    const result = db.parcels.insertOne(parcel);
    print("[Mongo] Parcel created with ID: " + result.insertedId);
    return result.insertedId;
}

function findParcelsBySender(senderId) {
    return db.parcels.find({ sender_id: senderId })
        .sort({ created_at: -1 })
        .toArray();
}

function findParcelsByStatus(status) {
    return db.parcels.find({ status: status })
        .sort({ created_at: -1 })
        .toArray();
}

function addFeatureToParcel(parcelId, feature) {
    const result = db.parcels.updateOne(
        { _id: parcelId },
        { $addToSet: { features: feature } }
    );
    print("[Mongo] Feature added to parcel " + parcelId + ": " + feature);
    return result.modifiedCount > 0;
}

function updateParcelStatus(parcelId, newStatus) {
    const result = db.parcels.updateOne(
        { _id: parcelId },
        { $set: { status: newStatus, updated_at: new Date() } }
    );
    print("[Mongo] Parcel " + parcelId + " status updated to: " + newStatus);
    return result.modifiedCount > 0;
}

// ============================================================================
// DELIVERY OPERATIONS
// ============================================================================

function createDelivery(parcelId, senderId, receiverId, fromAddress, toAddress) {
    const deliveryCount = db.deliveries.countDocuments() + 1;
    const trackingNumber = generateTrackingNumber(deliveryCount);
    const delivery = {
        _id: uuid(),
        parcel_id: parcelId,
        sender_id: senderId,
        receiver_id: receiverId,
        tracking_number: trackingNumber,
        status: "pending",
        from_address: fromAddress,
        to_address: toAddress,
        created_at: new Date(),
        delivered_at: null,
        events: [
            {
                status: "pending",
                timestamp: new Date(),
                location: fromAddress.split(",")[0].trim(),
                comment: "Delivery order created"
            }
        ]
    };
    const result = db.deliveries.insertOne(delivery);
    updateParcelStatus(parcelId, "assigned");
    print("[Mongo] Delivery created with tracking number: " + trackingNumber);
    return result.insertedId;
}

function findDeliveryByTrackingNumber(trackingNumber) {
    return db.deliveries.findOne({ tracking_number: trackingNumber });
}

function findDeliveriesBySender(senderId) {
    return db.deliveries.find({ sender_id: senderId })
        .sort({ created_at: -1 })
        .toArray();
}

function findDeliveriesByReceiver(receiverId) {
    return db.deliveries.find({ receiver_id: receiverId })
        .sort({ created_at: -1 })
        .toArray();
}

function findDeliveriesByStatus(status) {
    return db.deliveries.find({ status: status })
        .sort({ created_at: -1 })
        .toArray();
}

function updateDeliveryStatus(deliveryId, newStatus, location, comment) {
    const updateDoc = {
        $set: { status: newStatus, updated_at: new Date() },
        $push: {
            events: {
                status: newStatus,
                timestamp: new Date(),
                location: location,
                comment: comment || null
            }
        }
    };
    if (newStatus === "delivered") {
        updateDoc.$set.delivered_at = new Date();
    }
    const result = db.deliveries.updateOne({ _id: deliveryId }, updateDoc);
    print("[Mongo] Delivery " + deliveryId + " status updated to: " + newStatus);
    return result.modifiedCount > 0;
}

function addDeliveryEvent(deliveryId, status, location, comment) {
    const result = db.deliveries.updateOne(
        { _id: deliveryId },
        {
            $push: {
                events: {
                    status: status,
                    timestamp: new Date(),
                    location: location,
                    comment: comment || null
                }
            }
        }
    );
    print("[Mongo] Event added to delivery " + deliveryId);
    return result.modifiedCount > 0;
}

function getTrackingHistory(trackingNumber) {
    const delivery = db.deliveries.findOne(
        { tracking_number: trackingNumber },
        { events: 1, tracking_number: 1, status: 1, _id: 0 }
    );
    if (!delivery) {
        print("Delivery not found");
        return null;
    }
    print("\n=== Tracking History for " + trackingNumber + " ===");
    print("Current status: " + delivery.status);
    print("\nEvents:");
    delivery.events.forEach(function (event, index) {
        print("  " + (index + 1) + ". [" + event.timestamp + "] " +
            event.status.toUpperCase() + " - " + event.location);
        if (event.comment) print("     Comment: " + event.comment);
    });
    return delivery;
}

function cancelDelivery(deliveryId, reason) {
    const result = db.deliveries.updateOne(
        { _id: deliveryId, status: { $in: ["pending", "in_transit"] } },
        {
            $set: { status: "cancelled" },
            $push: {
                events: {
                    status: "cancelled",
                    timestamp: new Date(),
                    location: "System",
                    comment: reason || "Cancelled by user request"
                }
            }
        }
    );
    if (result.modifiedCount > 0) {
        print("[Mongo] Delivery " + deliveryId + " cancelled");
        const delivery = db.deliveries.findOne({ _id: deliveryId });
        if (delivery) {
            updateParcelStatus(delivery.parcel_id, "created");
        }
    }
    return result.modifiedCount > 0;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

function uuid() {
    return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function (c) {
        const r = Math.random() * 16 | 0;
        const v = c === "x" ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}

function generateTrackingNumber(deliveryId) {
    const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    let suffix = "";
    for (let i = 0; i < 6; i++) {
        suffix += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return "DEL-" + deliveryId + "-" + suffix;
}

// ============================================================================
// TEST QUERIES
// ============================================================================

print("\n=== Test Queries ===");

print("\n1. Parcels for sender 1:");
const parcels = findParcelsBySender(1);
printjson(parcels.map(p => ({ id: p._id, status: p.status, weight: p.weight })));

print("\n2. Deliveries for sender 1:");
const deliveriesBySender = findDeliveriesBySender(1);
printjson(deliveriesBySender.map(d => ({ tracking: d.tracking_number, status: d.status })));

print("\n3. Deliveries for receiver 2:");
const deliveriesByReceiver = findDeliveriesByReceiver(2);
printjson(deliveriesByReceiver.map(d => ({ tracking: d.tracking_number, status: d.status })));

print("\n4. Delivery by tracking number 'DEL-1-ABC123':");
const delivery = findDeliveryByTrackingNumber("DEL-1-ABC123");
if (delivery) {
    printjson({ tracking: delivery.tracking_number, status: delivery.status, events_count: delivery.events.length });
}

print("\n5. Deliveries with status 'in_transit':");
const inTransit = findDeliveriesByStatus("in_transit");
printjson(inTransit.map(d => ({ tracking: d.tracking_number, from: d.from_address, to: d.to_address })));

print("\n=== All queries loaded successfully ===");