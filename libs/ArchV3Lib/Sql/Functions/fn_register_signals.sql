CREATE OR REPLACE FUNCTION fn_register_signals
(
    p_signals signal_register_info[],
    p_created_utc BIGINT
)
RETURNS void
LANGUAGE sql
AS $$
    INSERT INTO signals
    (
        app_signal_id,
        hash,
        signal_type,
        bucket,
        created_utc
    )
    SELECT
        p.app_signal_id,
        p.hash,
        p.signal_type,
        p.bucket,
        p_created_utc
    FROM unnest(p_signals) AS p
    ON CONFLICT ON CONSTRAINT signals_app_signal_id_unique
    DO NOTHING;
$$;