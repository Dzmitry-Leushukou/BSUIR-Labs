DO $$
DECLARE i INTEGER;
BEGIN
    FOR i IN 1..10000 LOOP
        INSERT INTO MyTable (id, val)
        VALUES ((random() * i)::INTEGER, (i * random())::INTEGER);
    END LOOP;
END $$;

SELECT count(*) FROM MyTable;