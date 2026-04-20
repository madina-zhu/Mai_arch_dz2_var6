// Delivery Service API — MongoDB Aggregation Pipeline
// Запуск: docker exec -i mongodb_delivery mongosh -u admin -p password < aggregation.js

db = db.getSiblingDB("mongo_delivery_db");

print("Running aggregation: delivery statistics by city and status");

// ============================================================================
// AGGREGATION PIPELINE: Статистика доставок по городам
// ============================================================================

const pipeline = [
    // Стадия 1: Join с parcels для получения информации о посылках
    {
        $lookup: {
            from: "parcels",
            localField: "parcel_id",
            foreignField: "_id",
            as: "parcel_info"
        }
    },

    // Стадия 2: Разворачиваем массив parcel_info
    { $unwind: "$parcel_info" },

    // Стадия 3: Фильтруем только завершенные и активные доставки
    {
        $match: {
            $or: [
                { status: "delivered" },
                { status: "in_transit" },
                { status: "assigned" }
            ]
        }
    },

    // Стадия 4: Группировка по городу назначения и статусу
    {
        $group: {
            _id: {
                to_city: "$parcel_info.to_city",
                status: "$status"
            },
            count: { $sum: 1 },
            avg_weight: { $avg: "$parcel_info.weight" },
            avg_price: { $avg: "$parcel_info.price" },
            total_value: { $sum: "$parcel_info.price" },
            parcels: { $addToSet: "$parcel_info._id" }
        }
    },

    // Стадия 5: Проекция с читаемыми именами
    {
        $project: {
            _id: 0,
            city: "$_id.to_city",
            status: "$_id.status",
            totalDeliveries: "$count",
            averageWeightKg: { $round: ["$avg_weight", 2] },
            averagePriceRub: { $round: ["$avg_price", 0] },
            totalValueRub: { $round: ["$total_value", 0] },
            uniqueParcelsCount: { $size: "$parcels" }
        }
    },

    // Стадия 6: Сортировка по количеству доставок
    {
        $sort: {
            totalDeliveries: -1,
            city: 1
        }
    }
];

const results = db.deliveries.aggregate(pipeline).toArray();

print("\n=== Delivery Statistics by City and Status ===");
printjson(results);

// ============================================================================
// AGGREGATION PIPELINE 2: Эффективность курьеров
// ============================================================================

const courierEfficiencyPipeline = [
    {
        $match: {
            status: "delivered",
            actual_delivery: { $ne: null },
            pickup_time: { $ne: null }
        }
    },
    {
        $addFields: {
            delivery_time_hours: {
                $divide: [
                    { $subtract: ["$actual_delivery", "$pickup_time"] },
                    1000 * 60 * 60  // конвертация миллисекунд в часы
                ]
            }
        }
    },
    {
        $group: {
            _id: "$courier_id",
            total_deliveries: { $sum: 1 },
            avg_delivery_time_hours: { $avg: "$delivery_time_hours" },
            min_delivery_time_hours: { $min: "$delivery_time_hours" },
            max_delivery_time_hours: { $max: "$delivery_time_hours" }
        }
    },
    {
        $project: {
            _id: 0,
            courier_id: "$_id",
            total_deliveries: 1,
            avg_delivery_time_hours: { $round: ["$avg_delivery_time_hours", 1] },
            min_delivery_time_hours: { $round: ["$min_delivery_time_hours", 1] },
            max_delivery_time_hours: { $round: ["$max_delivery_time_hours", 1] }
        }
    },
    {
        $sort: { total_deliveries: -1 }
    }
];

const courierResults = db.deliveries.aggregate(courierEfficiencyPipeline).toArray();

print("\n=== Courier Efficiency Statistics ===");
printjson(courierResults);

print("\n=== Aggregation finished ===");