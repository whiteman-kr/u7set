CREATE OR REPLACE FUNCTION fn_get_active_archive_files()
RETURNS SETOF arch_file_info
LANGUAGE sql
STABLE
AS
$$
    SELECT
        af.arch_file_id,
        af.signal_id,

        s.hash,
        af.bucket,
        s.signal_type,

        af.file_name,

        af.created_utc,
        af.time_from_utc,
        af.time_to_utc,

        af.record_count,
        af.file_size,

        af.completed,
        af.compressed,
        af.deleted
    FROM archive_files AS af
    INNER JOIN signals AS s
        ON s.signal_id = af.signal_id
       AND s.bucket = af.bucket
    WHERE af.deleted = FALSE
      AND af.completed = FALSE;
$$;