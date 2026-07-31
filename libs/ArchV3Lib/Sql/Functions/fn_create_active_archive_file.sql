CREATE OR REPLACE FUNCTION fn_create_active_archive_file
(
    p_hash           BIGINT,
    p_file_name      TEXT,
    p_time_from_utc  BIGINT,
    p_created_utc    BIGINT
)
RETURNS arch_file_info
LANGUAGE plpgsql
AS
$$
DECLARE
    v_signal_id    BIGINT;
    v_signal_type  INTEGER;
    v_bucket       SMALLINT;

    v_result       arch_file_info;
BEGIN
    SELECT
        s.signal_id,
        s.signal_type,
        s.bucket
    INTO
        v_signal_id,
        v_signal_type,
        v_bucket
    FROM signals AS s
    WHERE s.hash = p_hash;

    IF NOT FOUND THEN
        RAISE EXCEPTION
            'Signal with hash % is not registered',
            p_hash
            USING ERRCODE = 'foreign_key_violation';
    END IF;

    INSERT INTO archive_files
    (
        signal_id,
        bucket,
        file_name,
        time_from_utc,
        time_to_utc,
        record_count,
        file_size,
        created_utc,
        completed,
        compressed,
        deleted
    )
    VALUES
    (
        v_signal_id,
        v_bucket,
        p_file_name,
        p_time_from_utc,
        0,
        0,
        0,
        p_created_utc,
        FALSE,
        FALSE,
        FALSE
    )
    RETURNING
        archive_files.arch_file_id,
        archive_files.signal_id,
        v_signal_type,
        p_hash,
        archive_files.bucket,
        archive_files.file_name,
        archive_files.time_from_utc,
        archive_files.time_to_utc,
        archive_files.record_count,
        archive_files.file_size,
        archive_files.created_utc,
        archive_files.completed,
        archive_files.compressed,
        archive_files.deleted
    INTO v_result;

    RETURN v_result;
END;
$$;