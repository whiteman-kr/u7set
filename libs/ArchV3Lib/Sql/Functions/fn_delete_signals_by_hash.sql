CREATE OR REPLACE FUNCTION fn_delete_signals_by_hash
(
    p_hashes BIGINT[]
)
RETURNS void
LANGUAGE plpgsql
AS $$
BEGIN
    DELETE FROM archive_files AS af
    USING signals AS s
    WHERE af.signal_id = s.signal_id
      AND s.hash = ANY(p_hashes);

    DELETE FROM signals AS s
    WHERE s.hash = ANY(p_hashes);
END;
$$;