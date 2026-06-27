BEGIN;

CREATE TABLE archive_info
(
    archive_info_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    schema_version INTEGER NOT NULL,
    archive_version INTEGER NOT NULL,

    description TEXT NOT NULL,
    applied_utc BIGINT NOT NULL
);

CREATE TABLE signals
(
    signal_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    app_signal_id TEXT NOT NULL,
    hash BIGINT NOT NULL,

    signal_type SMALLINT NOT NULL,
    bucket SMALLINT NOT NULL,

    created_utc BIGINT NOT NULL,

    CONSTRAINT signals_app_signal_id_unique UNIQUE (app_signal_id),
    CONSTRAINT signals_hash_unique UNIQUE (hash),

    CONSTRAINT signals_bucket_check
        CHECK (bucket >= 0 AND bucket <= 255)
);

CREATE TABLE archive_files
(
    archive_file_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    signal_id BIGINT NOT NULL,
    bucket SMALLINT NOT NULL,

    file_name TEXT NOT NULL,

    time_from_utc BIGINT NOT NULL,
    time_to_utc BIGINT NOT NULL DEFAULT 0,

    record_count BIGINT NOT NULL DEFAULT 0,
    file_size BIGINT NOT NULL DEFAULT 0,

    compressed BOOLEAN NOT NULL DEFAULT FALSE,
    deleted BOOLEAN NOT NULL DEFAULT FALSE,

    created_utc BIGINT NOT NULL,

    CONSTRAINT archive_files_signal_fk
        FOREIGN KEY (signal_id)
        REFERENCES signals(signal_id),

    CONSTRAINT archive_files_bucket_check
        CHECK (bucket >= 0 AND bucket <= 255)
)
PARTITION BY LIST (bucket);

CREATE INDEX archive_files_signal_time_idx
ON archive_files
(
    signal_id,
    time_from_utc
);

COMMIT;