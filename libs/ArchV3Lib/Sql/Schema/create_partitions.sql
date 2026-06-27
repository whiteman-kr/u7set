DO $$
BEGIN
    FOR i IN 0..255 LOOP
        EXECUTE format(
            'CREATE TABLE archive_files_%s PARTITION OF archive_files FOR VALUES IN (%s)',
            to_hex(i),
            i);
    END LOOP;
END $$;