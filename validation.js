// Delivery Service API — MongoDB Schema Validation
// Запуск: docker exec -i mongodb_delivery mongosh -u admin -p password < validation.js

db = db.getSiblingDB("mongo_delivery_db");

print("Starting MongoDB validation setup for Delivery Service...");

// ========================================================================
// PARCELS COLLECTION VALIDATION
// ========================================================================

const parcelsValidator = {
    $jsonSchema: {
        bsonType: "object",
        required: ["sender_id", "recipient_id", "type", "title", "from_city", "to_city", "weight", "price", "address", "details", "status"],
        properties: {
            _id: {
                bsonType: "string",
                description: "must be a string (UUID)"
            },
            sender_id: {
                bsonType: ["int", "long"],
                minimum: 1,
                description: "must be a positive integer (reference to Postgres User)"
            },
            recipient_id: {
                bsonType: ["int", "long"],
                minimum: 1,
                description: "must be a positive integer (reference to Postgres User)"
            },
            type: {
                bsonType: "string",
                enum: ["standard", "fragile", "oversized", "express"],
                description: "must be one of: standard, fragile, oversized, express"
            },
            title: {
                bsonType: "string",
                minLength: 3,
                maxLength: 200,
                description: "must be a string with length between 3 and 200 characters"
            },
            from_city: {
                bsonType: "string",
                pattern: "^[a-zA-Zа-яА-Я\\s\\-]+$",
                description: "must contain only letters, spaces, or hyphens"
            },
            to_city: {
                bsonType: "string",
                pattern: "^[a-zA-Zа-яА-Я\\s\\-]+$",
                description: "must contain only letters, spaces, or hyphens"
            },
            weight: {
                bsonType: ["double", "int", "long", "decimal"],
                minimum: 0.1,
                exclusiveMinimum: true,
                description: "must be a positive number (minimum 0.1 kg)"
            },
            price: {
                bsonType: ["double", "int", "long", "decimal"],
                minimum: 0,
                description: "must be a non-negative number"
            },
            status: {
                bsonType: "string",
                enum: ["pending", "in_transit", "delivered", "cancelled"],
                description: "must be one of: pending, in_transit, delivered, cancelled"
            },
            address: {
                bsonType: "object",
                description: "must be an object containing address information"
            },
            details: {
                bsonType: "object",
                description: "must be an object with parcel-specific details"
            },
            features: {
                bsonType: "array",
                items: { bsonType: "string" },
                description: "must be an array of strings"
            },
            created_at: {
                bsonType: "date",
                description: "must be a valid date"
            },
            updated_at: {
                bsonType: "date",
                description: "must be a valid date"
            }
        },
        additionalProperties: false
    }
};

// ========================================================================
// DELIVERIES COLLECTION VALIDATION
// ========================================================================

const deliveriesValidator = {
    $jsonSchema: {
        bsonType: "object",
        required: ["parcel_id", "courier_id", "pickup_time", "estimated_delivery", "tracking_history"],
        properties: {
            _id: {
                bsonType: "string",
                description: "must be a string (UUID)"
            },
            parcel_id: {
                bsonType: "string",
                description: "must be a string (UUID reference to parcels collection)"
            },
            courier_id: {
                bsonType: ["int", "long"],
                minimum: 1,
                description: "must be a positive integer"
            },
            pickup_time: {
                bsonType: "date",
                description: "must be a valid date"
            },
            estimated_delivery: {
                bsonType: "date",
                description: "must be a valid date"
            },
            actual_delivery: {
                bsonType: ["date", "null"],
                description: "must be a date or null"
            },
            status: {
                bsonType: "string",
                enum: ["pending", "assigned", "in_transit", "delivered", "cancelled"],
                description: "must be one of: pending, assigned, in_transit, delivered, cancelled"
            },
            tracking_history: {
                bsonType: "array",
                minItems: 1,
                items: {
                    bsonType: "object",
                    required: ["status", "timestamp", "location"],
                    properties: {
                        status: { bsonType: "string" },
                        timestamp: { bsonType: "date" },
                        location: { bsonType: "string" },
                        reason: { bsonType: "string" }
                    }
                },
                description: "must be a non-empty array of tracking events"
            },
            created_at: {
                bsonType: "date",
                description: "must be a valid date"
            }
        }
    }
};

// ========================================================================
// APPLY VALIDATION
// ========================================================================

// Apply validation to parcels collection
if (db.getCollectionNames().includes("parcels")) {
    db.runCommand({
        collMod: "parcels",
        validator: parcelsValidator,
        validationLevel: "strict",
        validationAction: "error"
    });
    print("Validation applied to existing 'parcels' collection");
} else {
    db.createCollection("parcels", {
        validator: parcelsValidator,
        validationLevel: "strict",
        validationAction: "error"
    });
    print("Collection 'parcels' created with validation");
}

// Apply validation to deliveries collection
if (db.getCollectionNames().includes("deliveries")) {
    db.runCommand({
        collMod: "deliveries",
        validator: deliveriesValidator,
        validationLevel: "strict",
        validationAction: "error"
    });
    print("Validation applied to existing 'deliveries' collection");
} else {
    db.createCollection("deliveries", {
        validator: deliveriesValidator,
        validationLevel: "strict",
        validationAction: "error"
    });
    print("Collection 'deliveries' created with validation");
}

// ========================================================================
// TEST VALIDATION (попытка вставить невалидные данные)
// ========================================================================

print("\n=== Testing validation ===");

// This should FAIL — negative weight
print("\nAttempting to insert invalid parcel (negative weight):");
try {
    db.parcels.insertOne({
        _id: crypto.randomUUID(),
        sender_id: 1,
        recipient_id: 2,
        type: "standard",
        title: "Test",
        from_city: "Moscow",
        to_city: "SPB",
        weight: -5,  // INVALID: negative weight
        price: 100,
        address: { street: "Test" },
        details: {},
        status: "pending",
        created_at: new Date()
    });
    print("ERROR: Invalid document was accepted!");
} catch (e) {
    print("SUCCESS: Invalid document rejected —", e.message);
}

// This should FAIL — invalid status
print("\nAttempting to insert invalid parcel (invalid status):");
try {
    db.parcels.insertOne({
        _id: crypto.randomUUID(),
        sender_id: 1,
        recipient_id: 2,
        type: "standard",
        title: "Test",
        from_city: "Moscow",
        to_city: "SPB",
        weight: 1.5,
        price: 100,
        address: { street: "Test" },
        details: {},
        status: "invalid_status",  // INVALID: not in enum
        created_at: new Date()
    });
    print("ERROR: Invalid document was accepted!");
} catch (e) {
    print("SUCCESS: Invalid document rejected —", e.message);
}

// This should SUCCEED
print("\nAttempting to insert valid parcel:");
try {
    db.parcels.insertOne({
        _id: crypto.randomUUID(),
        sender_id: 1,
        recipient_id: 2,
        type: "standard",
        title: "Valid Parcel",
        from_city: "Moscow",
        to_city: "Saint Petersburg",
        weight: 2.5,
        price: 500,
        address: { street: "Test St", house: "10" },
        details: { is_fragile: false },
        features: ["tracking"],
        status: "pending",
        created_at: new Date()
    });
    print("SUCCESS: Valid document was accepted!");
} catch (e) {
    print("ERROR: Valid document was rejected —", e.message);
}

print("\n=== MongoDB validation setup completed ===");