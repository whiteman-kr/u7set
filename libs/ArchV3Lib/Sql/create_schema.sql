CREATE TABLE archive_info
(
    archive_info_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    schema_version INTEGER NOT NULL,
    archive_version INTEGER NOT NULL,

    description TEXT NOT NULL,
    applied_utc BIGINT NOT NULL
);