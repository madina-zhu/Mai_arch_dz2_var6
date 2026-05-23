-- ============================================================================
-- Schema: Delivery Database (аналог CDEK)
-- Database: PostgreSQL 16
-- ============================================================================

DROP TABLE IF EXISTS deliveries CASCADE;
DROP TABLE IF EXISTS parcels CASCADE;
DROP TABLE IF EXISTS users CASCADE;

-- ============================================================================
-- Table: users
-- ============================================================================
CREATE TABLE IF NOT EXISTS users (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    login VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW() NOT NULL,
    
    CONSTRAINT chk_users_login_length CHECK (LENGTH(login) >= 3 AND LENGTH(login) <= 50),
    CONSTRAINT chk_users_password_length CHECK (LENGTH(password_hash) >= 6)
);

-- ============================================================================
-- Table: parcels
-- ============================================================================
CREATE TABLE IF NOT EXISTS parcels (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    sender_id BIGINT NOT NULL,
    weight DOUBLE PRECISION NOT NULL,
    dimensions VARCHAR(50) NOT NULL,
    declared_value DECIMAL(12,2) NOT NULL,
    description TEXT,
    status VARCHAR(20) DEFAULT 'created' NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW() NOT NULL,

    CONSTRAINT fk_parcels_sender 
        FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE,
    
    CONSTRAINT chk_parcels_weight CHECK (weight > 0),
    CONSTRAINT chk_parcels_declared_value CHECK (declared_value >= 0),
    CONSTRAINT chk_parcels_status CHECK (status IN ('created', 'assigned', 'in_transit', 'delivered'))
);

-- ============================================================================
-- Table: deliveries (без партиционирования)
-- ============================================================================
CREATE TABLE IF NOT EXISTS deliveries (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    parcel_id BIGINT NOT NULL,
    sender_id BIGINT NOT NULL,
    receiver_id BIGINT NOT NULL,
    tracking_number VARCHAR(50) NOT NULL UNIQUE,
    status VARCHAR(20) DEFAULT 'pending' NOT NULL,
    from_address TEXT NOT NULL,
    to_address TEXT NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW() NOT NULL,
    delivered_at TIMESTAMP WITH TIME ZONE,

    CONSTRAINT fk_deliveries_parcel 
        FOREIGN KEY (parcel_id) REFERENCES parcels(id) ON DELETE CASCADE,
    CONSTRAINT fk_deliveries_sender 
        FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_deliveries_receiver 
        FOREIGN KEY (receiver_id) REFERENCES users(id) ON DELETE CASCADE,
    
    CONSTRAINT chk_deliveries_status CHECK (status IN ('pending', 'in_transit', 'delivered', 'cancelled')),
    CONSTRAINT chk_deliveries_tracking_length CHECK (LENGTH(tracking_number) >= 10)
);

-- ============================================================================
-- Индексы
-- ============================================================================

CREATE INDEX IF NOT EXISTS idx_users_login ON users(login);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_deliveries_tracking ON deliveries(tracking_number);

-- GIN индексы для поиска по маске
CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE INDEX IF NOT EXISTS idx_users_first_trgm ON users USING gin (first_name gin_trgm_ops);
CREATE INDEX IF NOT EXISTS idx_users_last_trgm ON users USING gin (last_name gin_trgm_ops);

-- Индексы для посылок
CREATE INDEX IF NOT EXISTS idx_parcels_sender_id ON parcels(sender_id);
CREATE INDEX IF NOT EXISTS idx_parcels_sender_status ON parcels(sender_id, status);

-- Индексы для доставок
CREATE INDEX IF NOT EXISTS idx_deliveries_sender_id ON deliveries(sender_id);
CREATE INDEX IF NOT EXISTS idx_deliveries_receiver_id ON deliveries(receiver_id);
CREATE INDEX IF NOT EXISTS idx_deliveries_status_created ON deliveries(status, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_deliveries_parcel_id ON deliveries(parcel_id);

-- Покрывающий индекс
CREATE INDEX IF NOT EXISTS idx_deliveries_sender_covering ON deliveries(sender_id, created_at DESC) 
    INCLUDE (tracking_number, status, to_address);