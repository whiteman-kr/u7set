CREATE OR REPLACE FUNCTION fn_register_signals
(
    p_signals signal_register_info[],
    p_created_utc BIGINT
)
RETURNS void
LANGUAGE plpgsql
AS $$
DECLARE
    v_bad_count BIGINT;
BEGIN
    WITH parsed AS
    (
        SELECT
            p.app_signal_id,
            p.hash,
            p.signal_type,
            p.bucket
        FROM unnest(p_signals) AS p
    )
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
    FROM parsed AS p
    ON CONFLICT ON CONSTRAINT signals_app_signal_id_unique
    DO NOTHING;

    WITH parsed AS
    (
        SELECT
            p.app_signal_id,
            p.hash,
            p.signal_type,
            p.bucket
        FROM unnest(p_signals) AS p
    )
    SELECT COUNT(*)
    INTO v_bad_count
    FROM parsed AS p
    INNER JOIN signals AS s ON s.app_signal_id = p.app_signal_id
    WHERE s.hash <> p.hash
       OR s.signal_type <> p.signal_type
       OR s.bucket <> p.bucket;

    IF v_bad_count <> 0 THEN
        RAISE EXCEPTION 'registered signal metadata mismatch';
    END IF;
END;
$$;