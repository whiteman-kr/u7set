DO $$
BEGIN
    IF NOT EXISTS
    (
        SELECT 1
        FROM pg_type
        WHERE typname = 'signal_register_info'
    ) THEN
        CREATE TYPE signal_register_info AS
        (
            app_signal_id TEXT,
            hash BIGINT,
            signal_type SMALLINT,
            bucket SMALLINT
        );
    END IF;
END
$$;