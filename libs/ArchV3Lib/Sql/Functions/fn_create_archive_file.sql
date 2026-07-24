CREATE OR REPLACE FUNCTION fn_create_archive_file
(
    p_signal_id BIGINT,
    p_bucket SMALLINT,
    p_file_name TEXT,
    p_time_from_utc BIGINT,
    p_created_utc BIGINT
)
RETURNS BIGINT
LANGUAGE sql
AS $$
    INSERT INTO archive_files
    (
        signal_id,
        bucket,
        file_name,
        time_from_utc,
        created_utc
    )
    VALUES
    (
        p_signal_id,
        p_bucket,
        p_file_name,
        p_time_from_utc,
        p_created_utc
    )
    RETURNING archive_file_id;
$$;