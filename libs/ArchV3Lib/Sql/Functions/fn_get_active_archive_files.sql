CREATE OR REPLACE FUNCTION fn_get_active_archive_files()
RETURNS TABLE
(
    archive_file_id BIGINT,
    signal_id BIGINT,
    signal_type SMALLINT,
    hash BIGINT,
    bucket SMALLINT,
    file_name TEXT,
    time_from_utc BIGINT,
    time_to_utc BIGINT,
    record_count BIGINT,
    file_size BIGINT,
    created_utc BIGINT
)
LANGUAGE sql
STABLE
AS
$$
    SELECT
        af.archive_file_id,
        af.signal_id,
        s.signal_type,
        s.hash,
        af.bucket,
        af.file_name,
        af.time_from_utc,
        af.time_to_utc,
        af.record_count,
        af.file_size,
        af.created_utc
    FROM archive_files AS af
    INNER JOIN signals AS s
        ON s.signal_id = af.signal_id
       AND s.bucket = af.bucket
    WHERE af.deleted = FALSE
      AND af.completed = FALSE;
$$;