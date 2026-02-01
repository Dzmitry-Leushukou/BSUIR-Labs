CREATE OR REPLACE FUNCTION check_even_more()
RETURNS TEXT AS
$$
DECLARE
even_count INTEGER;
odd_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO even_count FROM MyTable WHERE val % 2 = 0;
    SELECT COUNT(*) INTO odd_count FROM MyTable WHERE val % 2 != 0;

    RAISE NOTICE 'even_count: %, odd_count: %', even_count, odd_count;

    IF even_count > odd_count THEN
        RETURN 'TRUE';
    ELSIF even_count < odd_count THEN
        RETURN 'FALSE';
    ELSE
        RETURN 'EQUAL';
    END IF;
END;
$$ LANGUAGE plpgsql;

SELECT check_even_more() AS result;