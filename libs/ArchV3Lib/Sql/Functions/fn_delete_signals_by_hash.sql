CREATE OR REPLACE FUNCTION fn_delete_signals_by_hash
(
    p_hashes BIGINT[]
)
RETURNS TABLE
(
    file_name TEXT
)
LANGUAGE plpgsql
AS $$
BEGIN
    RETURN QUERY
    WITH deleted_files AS
    (
        DELETE FROM arch_files AS af
        USING signals AS s
        WHERE af.signal_id = s.signal_id
          AND s.hash = ANY(p_hashes)
        RETURNING af.file_name
    ),
    deleted_signals AS
    (
        DELETE FROM signals AS s
        WHERE s.hash = ANY(p_hashes)
    )
    SELECT file_name
    FROM deleted_files;
END;
$$;