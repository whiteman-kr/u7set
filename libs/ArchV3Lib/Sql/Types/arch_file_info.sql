CREATE TYPE arch_file_info AS
(
    arch_file_id    BIGINT,
    signal_id       BIGINT,

    hash            BIGINT,
    bucket          SMALLINT,
    signal_type     INTEGER,

    file_name       TEXT,

    created_utc     BIGINT,
    time_from_utc   BIGINT,
    time_to_utc     BIGINT,

    record_count    BIGINT,
    file_size       BIGINT,

    completed       BOOLEAN,
    compressed      BOOLEAN,
    deleted         BOOLEAN
);