// Delivery Service API — MongoDB Seed Data
// Запуск: cat data.js | docker exec -i mongodb_delivery mongosh -u admin -p password

db = db.getSiblingDB("mongo_delivery_db");

print("Starting data seeding for Delivery Service...");

const parcelsCollection = db.parcels;
const deliveriesCollection = db.deliveries;

parcelsCollection.deleteMany({});
deliveriesCollection.deleteMany({});

function uuid() {
    return crypto.randomUUID();
}

// ========================================================================
// PARCELS DATA (посылки)
// ========================================================================

const parcels = [
    {
        _id: uuid(),
        sender_id: 1,
        recipient_id: 2,
        type: "standard",
        title: "Документы для подписания",
        from_city: "Moscow",
        to_city: "Saint Petersburg",
        address: { street: "Tverskaya", house: "10", office: "5" },
        weight: 0.5,
        price: 500,
        details: { is_fragile: false, needs_refrigeration: false, dimensions: { length: 30, width: 20, height: 5 } },
        features: ["express_available"],
        status: "pending",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 1,
        recipient_id: 3,
        type: "fragile",
        title: "Стеклянная ваза",
        from_city: "Moscow",
        to_city: "Kazan",
        address: { street: "Leninsky Prospekt", house: "45", flat: "301" },
        weight: 2.5,
        price: 1500,
        details: { is_fragile: true, requires_signature: true, packing_type: "bubble_wrap" },
        features: ["insurance", "priority"],
        status: "in_transit",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 2,
        recipient_id: 1,
        type: "oversized",
        title: "Велосипед горный",
        from_city: "Novosibirsk",
        to_city: "Moscow",
        address: { street: "Krasny Prospekt", house: "15" },
        weight: 15.0,
        price: 5000,
        details: { dimensions: { length: 150, width: 80, height: 30 }, requires_assembly: true },
        features: ["large_cargo"],
        status: "pending",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 3,
        recipient_id: 2,
        type: "express",
        title: "Срочные документы",
        from_city: "Saint Petersburg",
        to_city: "Moscow",
        address: { street: "Nevsky", house: "1", flat: "12" },
        weight: 0.8,
        price: 1200,
        details: { delivery_by: "2026-04-17T18:00:00Z", priority_level: 1 },
        features: ["same_day"],
        status: "delivered",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 1,
        recipient_id: 4,
        type: "standard",
        title: "Книги и учебники",
        from_city: "Moscow",
        to_city: "Yekaterinburg",
        address: { street: "Arbat", house: "25", flat: "10" },
        weight: 8.0,
        price: 800,
        details: { total_books: 12, fragile: false },
        features: [],
        status: "in_transit",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 4,
        recipient_id: 1,
        type: "fragile",
        title: "Фарфоровый сервиз",
        from_city: "Kazan",
        to_city: "Moscow",
        address: { street: "Baumana", house: "2", flat: "10" },
        weight: 4.0,
        price: 2500,
        details: { is_fragile: true, pieces_count: 24, insurance_value: 10000 },
        features: ["insurance", "fragile_sticker"],
        status: "pending",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 2,
        recipient_id: 5,
        type: "standard",
        title: "Одежда и обувь",
        from_city: "Saint Petersburg",
        to_city: "Nizhny Novgorod",
        address: { street: "Sadovaya", house: "8", flat: "45" },
        weight: 5.0,
        price: 600,
        details: { items_count: 15, seasonal: "summer" },
        features: [],
        status: "cancelled",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 5,
        recipient_id: 3,
        type: "express",
        title: "Медицинские анализы",
        from_city: "Chelyabinsk",
        to_city: "Moscow",
        address: { street: "Lenina", house: "50", flat: "1" },
        weight: 0.3,
        price: 2000,
        details: { requires_refrigeration: true, biohazard: false },
        features: ["priority", "temperature_control"],
        status: "delivered",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 3,
        recipient_id: 4,
        type: "oversized",
        title: "Мебельный гарнитур",
        from_city: "Moscow",
        to_city: "Samara",
        address: { street: "Profsoyuznaya", house: "100" },
        weight: 120.0,
        price: 15000,
        details: { dimensions: { length: 200, width: 90, height: 80 }, requires_assembly: true },
        features: ["large_cargo", "elevator_required"],
        status: "pending",
        created_at: new Date()
    },
    {
        _id: uuid(),
        sender_id: 4,
        recipient_id: 2,
        type: "standard",
        title: "Электроника",
        from_city: "Moscow",
        to_city: "Rostov-on-Don",
        address: { street: "Novy Arbat", house: "15" },
        weight: 3.2,
        price: 900,
        details: { items: ["laptop", "tablet"], insurance_value: 50000 },
        features: ["insurance", "signature_required"],
        status: "in_transit",
        created_at: new Date()
    }
];

parcelsCollection.insertMany(parcels);
const parcelIds = parcels.map(p => p._id);

// ========================================================================
// DELIVERIES DATA (доставки — логистические задания)
// ========================================================================

const deliveries = [
    {
        _id: uuid(),
        parcel_id: parcelIds[0],
        courier_id: 101,
        pickup_time: new Date("2026-04-15T10:00:00Z"),
        estimated_delivery: new Date("2026-04-16T18:00:00Z"),
        actual_delivery: null,
        status: "assigned",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-14T09:00:00Z"), location: "Moscow" },
            { status: "assigned", timestamp: new Date("2026-04-14T10:00:00Z"), location: "Moscow" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[1],
        courier_id: 102,
        pickup_time: new Date("2026-04-14T14:00:00Z"),
        estimated_delivery: new Date("2026-04-18T18:00:00Z"),
        actual_delivery: null,
        status: "in_transit",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-13T09:00:00Z"), location: "Moscow" },
            { status: "assigned", timestamp: new Date("2026-04-13T10:00:00Z"), location: "Moscow" },
            { status: "picked_up", timestamp: new Date("2026-04-14T14:30:00Z"), location: "Moscow" },
            { status: "in_transit", timestamp: new Date("2026-04-14T16:00:00Z"), location: "Tver" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[2],
        courier_id: 103,
        pickup_time: new Date("2026-04-16T11:00:00Z"),
        estimated_delivery: new Date("2026-04-25T18:00:00Z"),
        actual_delivery: null,
        status: "pending",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-15T09:00:00Z"), location: "Novosibirsk" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[3],
        courier_id: 104,
        pickup_time: new Date("2026-04-10T09:00:00Z"),
        estimated_delivery: new Date("2026-04-10T18:00:00Z"),
        actual_delivery: new Date("2026-04-10T17:30:00Z"),
        status: "delivered",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-09T09:00:00Z"), location: "Saint Petersburg" },
            { status: "assigned", timestamp: new Date("2026-04-09T10:00:00Z"), location: "Saint Petersburg" },
            { status: "picked_up", timestamp: new Date("2026-04-10T09:30:00Z"), location: "Saint Petersburg" },
            { status: "in_transit", timestamp: new Date("2026-04-10T11:00:00Z"), location: "Moscow" },
            { status: "delivered", timestamp: new Date("2026-04-10T17:30:00Z"), location: "Moscow" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[4],
        courier_id: 105,
        pickup_time: new Date("2026-04-14T15:00:00Z"),
        estimated_delivery: new Date("2026-04-20T18:00:00Z"),
        actual_delivery: null,
        status: "in_transit",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-13T09:00:00Z"), location: "Moscow" },
            { status: "picked_up", timestamp: new Date("2026-04-14T15:30:00Z"), location: "Moscow" },
            { status: "in_transit", timestamp: new Date("2026-04-14T17:00:00Z"), location: "Vladimir" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[5],
        courier_id: 106,
        pickup_time: new Date("2026-04-17T12:00:00Z"),
        estimated_delivery: new Date("2026-04-19T18:00:00Z"),
        actual_delivery: null,
        status: "pending",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-16T09:00:00Z"), location: "Kazan" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[6],
        courier_id: 107,
        pickup_time: new Date("2026-04-12T13:00:00Z"),
        estimated_delivery: new Date("2026-04-15T18:00:00Z"),
        actual_delivery: null,
        status: "cancelled",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-11T09:00:00Z"), location: "Saint Petersburg" },
            { status: "cancelled", timestamp: new Date("2026-04-11T14:00:00Z"), location: "Saint Petersburg", reason: "Sender request" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[7],
        courier_id: 108,
        pickup_time: new Date("2026-04-13T08:00:00Z"),
        estimated_delivery: new Date("2026-04-13T18:00:00Z"),
        actual_delivery: new Date("2026-04-13T16:00:00Z"),
        status: "delivered",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-12T09:00:00Z"), location: "Chelyabinsk" },
            { status: "assigned", timestamp: new Date("2026-04-12T10:00:00Z"), location: "Chelyabinsk" },
            { status: "picked_up", timestamp: new Date("2026-04-13T08:30:00Z"), location: "Chelyabinsk" },
            { status: "in_transit", timestamp: new Date("2026-04-13T10:00:00Z"), location: "Moscow" },
            { status: "delivered", timestamp: new Date("2026-04-13T16:00:00Z"), location: "Moscow" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[8],
        courier_id: 109,
        pickup_time: new Date("2026-04-18T09:00:00Z"),
        estimated_delivery: new Date("2026-04-30T18:00:00Z"),
        actual_delivery: null,
        status: "pending",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-17T09:00:00Z"), location: "Moscow" }
        ],
        created_at: new Date()
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[9],
        courier_id: 110,
        pickup_time: new Date("2026-04-14T10:00:00Z"),
        estimated_delivery: new Date("2026-04-18T18:00:00Z"),
        actual_delivery: null,
        status: "in_transit",
        tracking_history: [
            { status: "created", timestamp: new Date("2026-04-13T09:00:00Z"), location: "Moscow" },
            { status: "picked_up", timestamp: new Date("2026-04-14T10:30:00Z"), location: "Moscow" },
            { status: "in_transit", timestamp: new Date("2026-04-14T12:00:00Z"), location: "Tula" }
        ],
        created_at: new Date()
    }
];

deliveriesCollection.insertMany(deliveries);

print("Inserted " + parcels.length + " parcels");
print("Inserted " + deliveries.length + " deliveries");
print("=== Delivery Service Data seeding completed ===");