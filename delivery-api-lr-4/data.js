// ============================================================================
// Test Data for Delivery API MongoDB
// ============================================================================

db = db.getSiblingDB("delivery_db");

print("Starting data seeding for MongoDB...");

const parcelsCollection = db.parcels;
const deliveriesCollection = db.deliveries;

// Clear existing data
parcelsCollection.deleteMany({});
deliveriesCollection.deleteMany({});

// Helper function to generate UUID
function uuid() {
    return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function (c) {
        const r = Math.random() * 16 | 0;
        const v = c === "x" ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}

// Helper function to generate tracking number
function generateTrackingNumber(deliveryId) {
    const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    let suffix = "";
    for (let i = 0; i < 6; i++) {
        suffix += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return "DEL-" + deliveryId + "-" + suffix;
}

// ============================================================================
// PARCELS DATA (10 parcels)
// ============================================================================

const parcels = [
    {
        _id: uuid(),
        sender_id: 1,
        weight: 2.5,
        dimensions: "30x20x15",
        declared_value: 5000.00,
        description: "Books and documents",
        status: "created",
        created_at: new Date("2026-05-10T10:00:00Z"),
        features: ["fragile"]
    },
    {
        _id: uuid(),
        sender_id: 1,
        weight: 5.0,
        dimensions: "40x30x20",
        declared_value: 15000.00,
        description: "Laptop and accessories",
        status: "assigned",
        created_at: new Date("2026-05-11T11:00:00Z"),
        features: ["fragile", "electronic"]
    },
    {
        _id: uuid(),
        sender_id: 2,
        weight: 1.2,
        dimensions: "25x15x10",
        declared_value: 3000.00,
        description: "Clothing",
        status: "in_transit",
        created_at: new Date("2026-05-12T12:00:00Z"),
        features: []
    },
    {
        _id: uuid(),
        sender_id: 2,
        weight: 3.8,
        dimensions: "35x25x20",
        declared_value: 8000.00,
        description: "Dishes",
        status: "delivered",
        created_at: new Date("2026-05-13T13:00:00Z"),
        features: ["fragile"]
    },
    {
        _id: uuid(),
        sender_id: 3,
        weight: 10.0,
        dimensions: "50x40x30",
        declared_value: 25000.00,
        description: "Home appliances",
        status: "created",
        created_at: new Date("2026-05-14T14:00:00Z"),
        features: ["heavy", "fragile"]
    },
    {
        _id: uuid(),
        sender_id: 3,
        weight: 0.8,
        dimensions: "20x15x10",
        declared_value: 1000.00,
        description: "Cosmetics",
        status: "assigned",
        created_at: new Date("2026-05-15T15:00:00Z"),
        features: ["liquid"]
    },
    {
        _id: uuid(),
        sender_id: 4,
        weight: 7.5,
        dimensions: "45x35x25",
        declared_value: 12000.00,
        description: "Sports equipment",
        status: "in_transit",
        created_at: new Date("2026-05-16T16:00:00Z"),
        features: ["bulky"]
    },
    {
        _id: uuid(),
        sender_id: 5,
        weight: 2.0,
        dimensions: "30x20x15",
        declared_value: 4500.00,
        description: "Toys",
        status: "delivered",
        created_at: new Date("2026-05-17T17:00:00Z"),
        features: []
    },
    {
        _id: uuid(),
        sender_id: 6,
        weight: 4.2,
        dimensions: "40x30x20",
        declared_value: 9500.00,
        description: "Books",
        status: "created",
        created_at: new Date("2026-05-18T18:00:00Z"),
        features: []
    },
    {
        _id: uuid(),
        sender_id: 7,
        weight: 6.0,
        dimensions: "50x40x30",
        declared_value: 18000.00,
        description: "Electronics",
        status: "assigned",
        created_at: new Date("2026-05-19T19:00:00Z"),
        features: ["fragile", "electronic"]
    }
];

parcelsCollection.insertMany(parcels);
print("Inserted " + parcels.length + " parcels");

// Store parcel IDs for reference
const parcelIds = parcels.map(p => p._id);

// ============================================================================
// DELIVERIES DATA (10 deliveries)
// ============================================================================

const deliveries = [
    {
        _id: uuid(),
        parcel_id: parcelIds[0],
        sender_id: 1,
        receiver_id: 2,
        tracking_number: "DEL-001-ABC123",
        status: "delivered",
        from_address: "Moscow, Tverskaya 15",
        to_address: "Saint Petersburg, Nevsky 25",
        created_at: new Date("2026-05-10T10:30:00Z"),
        delivered_at: new Date("2026-05-12T16:00:00Z"),
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-10T10:30:00Z"),
                location: "Moscow",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-11T08:00:00Z"),
                location: "Moscow Sorting Center",
                comment: "Parcel picked up"
            },
            {
                status: "delivered",
                timestamp: new Date("2026-05-12T16:00:00Z"),
                location: "Saint Petersburg",
                comment: "Delivered to receiver"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[1],
        sender_id: 1,
        receiver_id: 3,
        tracking_number: "DEL-002-DEF456",
        status: "in_transit",
        from_address: "Moscow, Leninsky Prospekt 10",
        to_address: "Kazan, Bauman 30",
        created_at: new Date("2026-05-11T11:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-11T11:30:00Z"),
                location: "Moscow",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-12T09:00:00Z"),
                location: "Moscow Sorting Center",
                comment: "In transit to Kazan"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[2],
        sender_id: 2,
        receiver_id: 4,
        tracking_number: "DEL-003-GHI789",
        status: "delivered",
        from_address: "Novosibirsk, Krasny Prospekt 50",
        to_address: "Yekaterinburg, Lenina 12",
        created_at: new Date("2026-05-12T12:30:00Z"),
        delivered_at: new Date("2026-05-14T14:00:00Z"),
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-12T12:30:00Z"),
                location: "Novosibirsk",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-13T10:00:00Z"),
                location: "Novosibirsk Sorting Center",
                comment: "In transit"
            },
            {
                status: "delivered",
                timestamp: new Date("2026-05-14T14:00:00Z"),
                location: "Yekaterinburg",
                comment: "Delivered successfully"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[3],
        sender_id: 2,
        receiver_id: 5,
        tracking_number: "DEL-004-JKL012",
        status: "pending",
        from_address: "Yekaterinburg, Mira 8",
        to_address: "Moscow, Arbat 20",
        created_at: new Date("2026-05-13T13:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-13T13:30:00Z"),
                location: "Yekaterinburg",
                comment: "Delivery order created"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[4],
        sender_id: 3,
        receiver_id: 6,
        tracking_number: "DEL-005-MNO345",
        status: "in_transit",
        from_address: "Kazan, Kremlevskaya 5",
        to_address: "Novosibirsk, Sovetskaya 40",
        created_at: new Date("2026-05-14T14:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-14T14:30:00Z"),
                location: "Kazan",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-15T11:00:00Z"),
                location: "Kazan Sorting Center",
                comment: "In transit to Novosibirsk"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[5],
        sender_id: 3,
        receiver_id: 7,
        tracking_number: "DEL-006-PQR678",
        status: "pending",
        from_address: "Saint Petersburg, Nevsky 100",
        to_address: "Moscow, Tverskaya 15",
        created_at: new Date("2026-05-15T15:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-15T15:30:00Z"),
                location: "Saint Petersburg",
                comment: "Delivery order created"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[6],
        sender_id: 4,
        receiver_id: 8,
        tracking_number: "DEL-007-STU901",
        status: "delivered",
        from_address: "Moscow, Kutuzovsky Prospekt 30",
        to_address: "Sochi, Kurortny 15",
        created_at: new Date("2026-05-16T16:30:00Z"),
        delivered_at: new Date("2026-05-18T12:00:00Z"),
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-16T16:30:00Z"),
                location: "Moscow",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-17T09:00:00Z"),
                location: "Moscow Sorting Center",
                comment: "In transit to Sochi"
            },
            {
                status: "delivered",
                timestamp: new Date("2026-05-18T12:00:00Z"),
                location: "Sochi",
                comment: "Delivered to receiver"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[7],
        sender_id: 5,
        receiver_id: 9,
        tracking_number: "DEL-008-VWX234",
        status: "cancelled",
        from_address: "Yekaterinburg, Lenina 50",
        to_address: "Moscow, Mira 10",
        created_at: new Date("2026-05-17T17:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-17T17:30:00Z"),
                location: "Yekaterinburg",
                comment: "Delivery order created"
            },
            {
                status: "cancelled",
                timestamp: new Date("2026-05-18T10:00:00Z"),
                location: "Yekaterinburg",
                comment: "Cancelled by sender"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[8],
        sender_id: 6,
        receiver_id: 10,
        tracking_number: "DEL-009-YZA567",
        status: "pending",
        from_address: "Novosibirsk, Dmitriya Donskogo 12",
        to_address: "Kazan, Yapeeva 8",
        created_at: new Date("2026-05-18T18:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-18T18:30:00Z"),
                location: "Novosibirsk",
                comment: "Delivery order created"
            }
        ]
    },
    {
        _id: uuid(),
        parcel_id: parcelIds[9],
        sender_id: 7,
        receiver_id: 1,
        tracking_number: "DEL-010-BCD890",
        status: "in_transit",
        from_address: "Sochi, Kurortny 5",
        to_address: "Moscow, Tverskaya 15",
        created_at: new Date("2026-05-19T19:30:00Z"),
        delivered_at: null,
        events: [
            {
                status: "created",
                timestamp: new Date("2026-05-19T19:30:00Z"),
                location: "Sochi",
                comment: "Delivery order created"
            },
            {
                status: "in_transit",
                timestamp: new Date("2026-05-20T08:00:00Z"),
                location: "Sochi Sorting Center",
                comment: "In transit to Moscow"
            }
        ]
    }
];

deliveriesCollection.insertMany(deliveries);
print("Inserted " + deliveries.length + " deliveries");

print("=== Data seeding completed ===");
print("Data inserted:");
print("  - Parcels: " + parcels.length);
print("  - Deliveries: " + deliveries.length);