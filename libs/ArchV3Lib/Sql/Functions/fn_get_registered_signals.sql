CREATE OR REPLACE FUNCTION fn_get_registered_signals()
RETURNS TABLE
(
    signal_id BIGINT,
    signal_type SMALLINT,
    app_signal_id TEXT,
    hash BIGINT,
    bucket SMALLINT,
    created_utc BIGINT
)
LANGUAGE sql
STABLE
AS $$
    SELECT
        s.signal_id,
        s.signal_type,
        s.app_signal_id,
        s.hash,
        s.bucket,
        s.created_utc
    FROM signals AS s
    ORDER BY s.signal_id;
$$;