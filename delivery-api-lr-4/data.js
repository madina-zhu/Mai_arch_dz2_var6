db = db.getSiblingDB("mongo_delivery_db");

print("Starting data seeding...");

const parcelsCollection = db.parcels;
const deliveriesCollection = db.deliveries;

parcelsCollection.deleteMany({});
deliveriesCollection.deleteMany({});

function uuid() {
    return crypto.randomUUID();
}

// ========================================================================
// PROPERTIES DATA
// ========================================================================

const parcels = [
    {
        _id: uuid(),
        owner_id: 1,
        type: "apartment",
        title: "Уютная двушка в центре",
        city: "Moscow",
        address: { street: "Tverskaya", house: "10", flat: "5" },
        price: 15000000,
        details: { rooms: 2, floor: 5, total_floors: 9, has_elevator: true },
        features: ["balcony", "internet"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 1,
        type: "house",
        title: "Дом за городом с баней",
        city: "Khimki",
        address: { street: "Lesnaya", house: "1" },
        price: 25000000,
        details: { area_sqm: 150, land_area_sqm: 600, has_garage: true, floors: 2 },
        features: ["sauna", "garden", "garage"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 2,
        type: "commercial",
        title: "Офисное помещение IT",
        city: "Moscow",
        address: { street: "Leninsky Prospekt", house: "45", flat: "Office 301" },
        price: 500000,
        details: { purpose: "office", area_sqm: 80, floor: 3 },
        features: ["parking", "security", "metro_nearby"],
        status: "rented",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 1,
        type: "apartment",
        title: "Студия на Невском",
        city: "St Petersburg",
        address: { street: "Nevsky", house: "1", flat: "12" },
        price: 8000000,
        details: { rooms: 1, floor: 2, total_floors: 5, has_elevator: false },
        features: [],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 2,
        type: "house",
        title: "Коттедж у моря",
        city: "Sochi",
        address: { street: "Kurortny", house: "5" },
        price: 40000000,
        details: { area_sqm: 200, land_area_sqm: 500, has_pool: true, floors: 3 },
        features: ["pool", "sea_view"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 1,
        type: "commercial",
        title: "Складское помещение",
        city: "Moscow",
        address: { street: "Industrial", house: "10" },
        price: 100000,
        details: { purpose: "warehouse", area_sqm: 500, ceiling_height: 6 },
        features: ["loading_zone"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 2,
        type: "apartment",
        title: "Трешка в Казани",
        city: "Kazan",
        address: { street: "Baumana", house: "2", flat: "10" },
        price: 12000000,
        details: { rooms: 3, floor: 4, total_floors: 9, has_elevator: true },
        features: ["balcony", "renovated"],
        status: "sold",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 1,
        type: "apartment",
        title: "Лофт в Артплее",
        city: "Moscow",
        address: { street: "ArtPlay", house: "1", flat: "404" },
        price: 20000000,
        details: { rooms: 2, floor: 4, total_floors: 4, has_elevator: true, style: "loft" },
        features: ["design", "high_ceilings"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 2,
        type: "land",
        title: "Земельный участок ИЖС",
        city: "Tula",
        address: { street: "Village", house: "15" },
        price: 5000000,
        details: { area_sqm: 1000, zoning: "residential", has_communications: true },
        features: ["fence", "electricity"],
        status: "active",
        created_at: new Date()
    },

    {
        _id: uuid(),
        owner_id: 1,
        type: "commercial",
        title: "Магазин на первой линии",
        city: "Novosibirsk",
        address: { street: "Lenina", house: "50", flat: "1" },
        price: 300000,
        details: { purpose: "retail", area_sqm: 60, floor: 1 },
        features: ["first_floor", "showcase"],
        status: "active",
        created_at: new Date()
    }
];

parcelsCollection.insertMany(properties);
const parcelIds = properties.map(p => p._id);

// ========================================================================
// VIEWINGS DATA
// ========================================================================

const deliveries = [
    {
        _id: uuid(),
        property_id: parcelIds[0],
        user_id: 2,
        scheduled_time: new Date("2026-04-15T10:00:00Z"),
        status: "confirmed",
        comments: [{ text: "Буду с риелтором", author: "user", timestamp: new Date() }],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[1],
        user_id: 1,
        scheduled_time: new Date("2026-04-16T14:00:00Z"),
        status: "pending",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[2],
        user_id: 1,
        scheduled_time: new Date("2026-04-10T12:00:00Z"),
        status: "completed",
        comments: [{ text: "Объект понравился", author: "agent", timestamp: new Date() }],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[3],
        user_id: 2,
        scheduled_time: new Date("2026-04-20T10:00:00Z"),
        status: "pending",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[4],
        user_id: 1,
        scheduled_time: new Date("2026-04-11T10:00:00Z"),
        status: "cancelled",
        comments: [{ text: "Передумал", author: "user", timestamp: new Date() }],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[5],
        user_id: 2,
        scheduled_time: new Date("2026-04-12T10:00:00Z"),
        status: "confirmed",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[6],
        user_id: 1,
        scheduled_time: new Date("2026-04-13T10:00:00Z"),
        status: "pending",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[7],
        user_id: 2,
        scheduled_time: new Date("2026-04-14T10:00:00Z"),
        status: "confirmed",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[8],
        user_id: 1,
        scheduled_time: new Date("2026-04-15T10:00:00Z"),
        status: "pending",
        comments: [],
        created_at: new Date()
    },

    {
        _id: uuid(),
        property_id: parcelIds[9],
        user_id: 2,
        scheduled_time: new Date("2026-04-16T10:00:00Z"),
        status: "pending",
        comments: [],
        created_at: new Date()
    }
];

deliveriesCollection.insertMany(viewings);

print("Inserted " + properties.length + " properties");
print("Inserted " + viewings.length + " viewings");
print("=== Data seeding completed ===");