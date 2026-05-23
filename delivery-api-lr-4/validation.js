// ============================================================================
// MongoDB Validation Schemas for Delivery API
// ============================================================================

db = db.getSiblingDB("delivery_db");

print("Starting MongoDB validation setup...");

// ============================================================================
// PARCELS COLLECTION VALIDATION
// ============================================================================

const parcelsValidator = {
    $jsonSchema: {
        bsonType: "object",
        required: ["sender_id", "weight", "dimensions", "declared_value", "status", "created_at"],
        properties: {
            _id: {
                bsonType: "string",
                description: "must be a string (UUID) and is required"
            },
            sender_id: {
                bsonType: ["int", "long"],
                description: "must be an integer and is required (Reference to PostgreSQL users)"
            },
            weight: {
                bsonType: ["double", "int", "long", "decimal"],
                minimum: 0.01,
                description: "must be a positive number (weight in kg)"
            },
            dimensions: {
                bsonType: "string",
                pattern: "^\\d+x\\d+x\\d+$",
                description: "must be in format '30x20x15'"
            },
            declared_value: {
                bsonType: ["double", "int", "long", "decimal"],
                minimum: 0,
                description: "must be a non-negative number"
            },
            description: {
                bsonType: "string",
                maxLength: 500,
                description: "optional description of parcel contents"
            },
            status: {
                bsonType: "string",
                enum: ["created", "assigned", "in_transit", "delivered"],
                description: "must be one of allowed values"
            },
            created_at: {
                bsonType: "date",
                description: "must be a date"
            },
            features: {
                bsonType: "array",
                items: { bsonType: "string" },
                description: "optional array of features (e.g., fragile, express)"
            }
        }
    }
};

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

// ============================================================================
// DELIVERIES COLLECTION VALIDATION
// ============================================================================

const deliveriesValidator = {
    $jsonSchema: {
        bsonType: "object",
        required: ["parcel_id", "sender_id", "receiver_id", "tracking_number", "status", "from_address", "to_address", "created_at", "events"],
        properties: {
            _id: {
                bsonType: "string",
                description: "must be a string (UUID)"
            },
            parcel_id: {
                bsonType: "string",
                description: "must be a string (UUID reference to parcels collection)"
            },
            sender_id: {
                bsonType: ["int", "long"],
                description: "must be integer (Reference to PostgreSQL users)"
            },
            receiver_id: {
                bsonType: ["int", "long"],
                description: "must be integer (Reference to PostgreSQL users)"
            },
            tracking_number: {
                bsonType: "string",
                pattern: "^DEL-\\d+-[A-Z0-9]{6}$",
                description: "must be in format 'DEL-123-ABC456'"
            },
            status: {
                bsonType: "string",
                enum: ["pending", "in_transit", "delivered", "cancelled"],
                description: "must be one of allowed values"
            },
            from_address: {
                bsonType: "string",
                minLength: 5,
                description: "sender address"
            },
            to_address: {
                bsonType: "string",
                minLength: 5,
                description: "receiver address"
            },
            created_at: {
                bsonType: "date",
                description: "delivery creation timestamp"
            },
            delivered_at: {
                bsonType: ["date", "null"],
                description: "timestamp when delivery completed"
            },
            events: {
                bsonType: "array",
                items: {
                    bsonType: "object",
                    required: ["status", "timestamp", "location"],
                    properties: {
                        status: {
                            bsonType: "string",
                            description: "event status"
                        },
                        timestamp: {
                            bsonType: "date",
                            description: "event timestamp"
                        },
                        location: {
                            bsonType: "string",
                            description: "event location"
                        },
                        comment: {
                            bsonType: "string",
                            description: "optional comment"
                        }
                    }
                },
                description: "tracking events history"
            }
        }
    }
};

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

// ============================================================================
// CREATE INDEXES
// ============================================================================

// Parcels indexes
db.parcels.createIndex({ "sender_id": 1 });
db.parcels.createIndex({ "sender_id": 1, "created_at": -1 });
db.parcels.createIndex({ "status": 1 });

// Deliveries indexes
db.deliveries.createIndex({ "tracking_number": 1 }, { unique: true });
db.deliveries.createIndex({ "sender_id": 1 });
db.deliveries.createIndex({ "receiver_id": 1 });
db.deliveries.createIndex({ "sender_id": 1, "created_at": -1 });
db.deliveries.createIndex({ "receiver_id": 1, "created_at": -1 });
db.deliveries.createIndex({ "status": 1, "created_at": -1 });
db.deliveries.createIndex({ "parcel_id": 1 });

print("=== MongoDB validation setup completed ===");
print("Indexes created:");
printjson(db.parcels.getIndexes());
printjson(db.deliveries.getIndexes());