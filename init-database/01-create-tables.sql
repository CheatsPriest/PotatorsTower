CREATE TYPE server_state AS ENUM ('up', 'down', 'unstable', 'content_mismatch');

CREATE TABLE servers (
    id BIGSERIAL PRIMARY KEY,
    endpoint TEXT NOT NULL,
    status server_state,
    last_ping TIMESTAMP,
    delay REAL,
    content TEXT,
    content_type TEXT,
    priority SMALLINT,
    cheack_ssl BOOLEAN,
    is_https BOOLEAN,
    path TEXT,
    load_media BOOLEAN
);

CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    login TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    email TEXT,
    tg_tag TEXT,-- UNIQUE??
    chat_id BIGINT-- UNIQUE??
);

CREATE TABLE user_subscriptions (
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    site_id BIGINT NOT NULL REFERENCES servers(id) ON DELETE CASCADE,
    PRIMARY KEY (user_id, site_id)
);

CREATE TABLE down_servers (
    id BIGSERIAL PRIMARY KEY,
    server_id BIGINT NOT NULL REFERENCES servers(id) ON DELETE CASCADE,
    endpoint TEXT NOT NULL
);

CREATE TABLE logs (
    id BIGSERIAL PRIMARY KEY,
    server_id BIGINT NOT NULL REFERENCES servers(id) ON DELETE CASCADE,
    date_time TIMESTAMP NOT NULL,
    status server_state NOT NULL,
    delay REAL,
    status_code SMALLINT NOT NULL,
    error TEXT
);

CREATE TABLE new_servers (
    id BIGSERIAL PRIMARY KEY,
    endpoint TEXT NOT NULL
);

CREATE INDEX idx_servers_status ON servers(status);
CREATE INDEX idx_servers_endpoint ON servers(endpoint);
CREATE INDEX idx_users_login ON users(login);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_user_subscriptions_user_id ON user_subscriptions(user_id);
CREATE INDEX idx_user_subscriptions_site_id ON user_subscriptions(site_id);
CREATE INDEX idx_down_servers_server_id ON down_servers(server_id);
CREATE INDEX idx_logs_server_id ON logs(server_id);
CREATE INDEX idx_logs_date_time ON logs(date_time);
