DO $$
BEGIN
    FOR i IN 0..255 LOOP
        EXECUTE format(
            'CREATE TABLE arch_files_%s PARTITION OF arch_files FOR VALUES IN (%s)',
            to_hex(i),
            i);
    END LOOP;
END $$;