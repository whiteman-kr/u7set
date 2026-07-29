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

    signal_type SMALLINT NOT NULL,

    app_signal_id TEXT NOT NULL,

    hash BIGINT NOT NULL,
    bucket SMALLINT NOT NULL,

    created_utc BIGINT NOT NULL,

    CONSTRAINT signals_app_signal_id_unique 
        UNIQUE (app_signal_id),

    CONSTRAINT signals_hash_unique 
        UNIQUE (hash),

    CONSTRAINT signals_signal_bucket_unique
        UNIQUE (signal_id, bucket),

    CONSTRAINT signals_bucket_check
        CHECK (bucket >= 0 AND bucket <= 255)
);

CREATE TABLE archive_files
(
    archive_file_id BIGINT GENERATED ALWAYS AS IDENTITY,

    signal_id BIGINT NOT NULL,
    bucket SMALLINT NOT NULL,

    file_name TEXT NOT NULL,

    time_from_utc BIGINT NOT NULL,
    time_to_utc BIGINT NOT NULL DEFAULT 0,

    record_count BIGINT NOT NULL DEFAULT 0,
    file_size BIGINT NOT NULL DEFAULT 0,

    completed BOOLEAN NOT NULL DEFAULT FALSE,
    compressed BOOLEAN NOT NULL DEFAULT FALSE,
    deleted BOOLEAN NOT NULL DEFAULT FALSE,

    created_utc BIGINT NOT NULL,

    CONSTRAINT archive_files_pk
        PRIMARY KEY (bucket, archive_file_id),

    CONSTRAINT archive_files_signal_fk
        FOREIGN KEY (signal_id, bucket)
        REFERENCES signals(signal_id, bucket),

    CONSTRAINT archive_files_bucket_check
        CHECK (bucket >= 0 AND bucket <= 255),
    
    CONSTRAINT archive_files_time_check
        CHECK (time_to_utc = 0 OR time_to_utc >= time_from_utc),
   
    CONSTRAINT archive_files_record_count_check
        CHECK (record_count >= 0),

    CONSTRAINT archive_files_file_size_check
       CHECK (file_size >= 0),

    CONSTRAINT archive_files_completed_time_check
       CHECK (NOT completed OR time_to_utc > 0),

    CONSTRAINT archive_files_compressed_check
        CHECK (NOT compressed OR completed)
)
PARTITION BY LIST (bucket);

CREATE INDEX archive_files_signal_time_idx
ON archive_files
(
    signal_id,
    time_from_utc
);

CREATE UNIQUE INDEX archive_files_signal_active_idx
ON archive_files
(
    signal_id,
    bucket
)
WHERE deleted = FALSE
  AND completed = FALSE;