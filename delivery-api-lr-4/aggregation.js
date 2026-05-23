// ============================================================================
// MongoDB Aggregation Pipeline for Delivery Analytics
// ============================================================================

db = db.getSiblingDB("delivery_db");

print("=== Running Aggregation Pipelines ===\n");

// ============================================================================
// Aggregation 1: Delivery Statistics by Status
// ============================================================================

print("1. Delivery Statistics by Status:");
print("=================================");

const statusStats = db.deliveries.aggregate([
    {
        $group: {
            _id: "$status",
            count: { $sum: 1 },
            avg_delivery_time_hours: {
                $avg: {
                    $cond: [
                        { $ne: ["$delivered_at", null] },
                        { $divide: [{ $subtract: ["$delivered_at", "$created_at"] }, 3600000] },
                        null
                    ]
                }
            }
        }
    },
    {
        $project: {
            status: "$_id",
            count: 1,
            avg_delivery_time_hours: { $round: ["$avg_delivery_time_hours", 2] },
            _id: 0
        }
    },
    {
        $sort: { count: -1 }
    }
]).toArray();

printjson(statusStats);

// ============================================================================
// Aggregation 2: Top Senders by Delivery Count
// ============================================================================

print("\n2. Top Senders by Delivery Count:");
print("==================================");

const topSenders = db.deliveries.aggregate([
    {
        $group: {
            _id: "$sender_id",
            delivery_count: { $sum: 1 },
            completed_count: {
                $sum: { $cond: [{ $eq: ["$status", "delivered"] }, 1, 0] }
            }
        }
    },
    {
        $sort: { delivery_count: -1 }
    },
    {
        $limit: 5
    },
    {
        $project: {
            sender_id: "$_id",
            delivery_count: 1,
            completed_count: 1,
            completion_rate: {
                $round: [
                    { $multiply: [{ $divide: ["$completed_count", "$delivery_count"] }, 100] },
                    2
                ]
            },
            _id: 0
        }
    }
]).toArray();

printjson(topSenders);

// ============================================================================
// Aggregation 3: Daily Delivery Volume (last 10 days)
// ============================================================================

print("\n3. Daily Delivery Volume (last 10 days):");
print("=========================================");

const tenDaysAgo = new Date();
tenDaysAgo.setDate(tenDaysAgo.getDate() - 10);

const dailyVolume = db.deliveries.aggregate([
    {
        $match: {
            created_at: { $gte: tenDaysAgo }
        }
    },
    {
        $group: {
            _id: {
                $dateToString: { format: "%Y-%m-%d", date: "$created_at" }
            },
            count: { $sum: 1 },
            deliveries: { $push: "$tracking_number" }
        }
    },
    {
        $project: {
            date: "$_id",
            count: 1,
            sample_deliveries: { $slice: ["$deliveries", 3] },
            _id: 0
        }
    },
    {
        $sort: { date: -1 }
    }
]).toArray();

printjson(dailyVolume);

// ============================================================================
// Aggregation 4: Parcel Weight Distribution
// ============================================================================

print("\n4. Parcel Weight Distribution:");
print("==============================");

const weightDistribution = db.parcels.aggregate([
    {
        $bucket: {
            groupBy: "$weight",
            boundaries: [0, 1, 2, 5, 10, 20, 50],
            default: "50+",
            output: {
                count: { $sum: 1 },
                avg_declared_value: { $avg: "$declared_value" },
                parcels: { $push: "$_id" }
            }
        }
    },
    {
        $project: {
            weight_range: {
                $switch: {
                    branches: [
                        { case: { $eq: ["$_id", 0] }, then: "0-1 kg" },
                        { case: { $eq: ["$_id", 1] }, then: "1-2 kg" },
                        { case: { $eq: ["$_id", 2] }, then: "2-5 kg" },
                        { case: { $eq: ["$_id", 5] }, then: "5-10 kg" },
                        { case: { $eq: ["$_id", 10] }, then: "10-20 kg" },
                        { case: { $eq: ["$_id", 20] }, then: "20-50 kg" }
                    ],
                    default: "50+ kg"
                }
            },
            count: 1,
            avg_declared_value: { $round: ["$avg_declared_value", 2] },
            _id: 0
        }
    }
]).toArray();

printjson(weightDistribution);

// ============================================================================
// Aggregation 5: Average Delivery Time by Route (from_address -> to_address)
// ============================================================================

print("\n5. Average Delivery Time by Route:");
print("===================================");

const routeStats = db.deliveries.aggregate([
    {
        $match: {
            status: "delivered",
            delivered_at: { $ne: null }
        }
    },
    {
        $group: {
            _id: {
                from: "$from_address",
                to: "$to_address"
            },
            count: { $sum: 1 },
            avg_delivery_hours: {
                $avg: {
                    $divide: [{ $subtract: ["$delivered_at", "$created_at"] }, 3600000]
                }
            },
            min_delivery_hours: {
                $min: {
                    $divide: [{ $subtract: ["$delivered_at", "$created_at"] }, 3600000]
                }
            },
            max_delivery_hours: {
                $max: {
                    $divide: [{ $subtract: ["$delivered_at", "$created_at"] }, 3600000]
                }
            }
        }
    },
    {
        $project: {
            from_address: "$_id.from",
            to_address: "$_id.to",
            delivery_count: "$count",
            avg_hours: { $round: ["$avg_delivery_hours", 2] },
            min_hours: { $round: ["$min_delivery_hours", 2] },
            max_hours: { $round: ["$max_delivery_hours", 2] },
            _id: 0
        }
    },
    {
        $sort: { delivery_count: -1 }
    },
    {
        $limit: 10
    }
]).toArray();

printjson(routeStats);

// ============================================================================
// Aggregation 6: Delivery Events Timeline (Lookup)
// ============================================================================

print("\n6. Unwind Events Timeline:");
print("===========================");

const eventsTimeline = db.deliveries.aggregate([
    {
        $match: {
            status: { $in: ["in_transit", "delivered"] }
        }
    },
    {
        $unwind: "$events"
    },
    {
        $group: {
            _id: {
                date: { $dateToString: { format: "%Y-%m-%d", date: "$events.timestamp" } },
                status: "$events.status"
            },
            count: { $sum: 1 }
        }
    },
    {
        $project: {
            date: "$_id.date",
            status: "$_id.status",
            event_count: "$count",
            _id: 0
        }
    },
    {
        $sort: { date: -1, status: 1 }
    }
]).toArray();

printjson(eventsTimeline);

// ============================================================================
// Aggregation 7: Parcels with Multiple Features (using $size)
// ============================================================================

print("\n7. Parcels with Multiple Features:");
print("===================================");

const parcelsWithFeatures = db.parcels.aggregate([
    {
        $match: {
            features: { $exists: true, $ne: [] }
        }
    },
    {
        $addFields: {
            feature_count: { $size: "$features" }
        }
    },
    {
        $match: {
            feature_count: { $gte: 1 }
        }
    },
    {
        $project: {
            _id: 1,
            weight: 1,
            feature_count: 1,
            features: 1
        }
    },
    {
        $sort: { feature_count: -1, weight: -1 }
    }
]).toArray();

printjson(parcelsWithFeatures);

// ============================================================================
// Aggregation 8: Expensive Parcels (declared_value > 10000)
// ============================================================================

print("\n8. Expensive Parcels (value > 10000):");
print("======================================");

const expensiveParcels = db.parcels.aggregate([
    {
        $match: {
            declared_value: { $gt: 10000 }
        }
    },
    {
        $lookup: {
            from: "deliveries",
            localField: "_id",
            foreignField: "parcel_id",
            as: "delivery_info"
        }
    },
    {
        $project: {
            _id: 1,
            weight: 1,
            declared_value: 1,
            description: 1,
            delivery_status: { $arrayElemAt: ["$delivery_info.status", 0] },
            tracking_number: { $arrayElemAt: ["$delivery_info.tracking_number", 0] }
        }
    },
    {
        $sort: { declared_value: -1 }
    }
]).toArray();

printjson(expensiveParcels);

print("\n=== All Aggregations Completed ===");