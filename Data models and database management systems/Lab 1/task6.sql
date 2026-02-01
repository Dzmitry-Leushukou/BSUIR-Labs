CREATE OR REPLACE FUNCTION calculate_reward(salary NUMERIC, bonus INTEGER)
RETURNS NUMERIC AS
$$
DECLARE
result NUMERIC;
BEGIN
    IF bonus IS NULL OR bonus < 0 THEN
        RAISE EXCEPTION 'Incorrect bonus value (%): Value must be non-negative', bonus;
    END IF;
    if salary IS NULL OR salary < 0 THEN
        RAISE EXCEPTION 'Incorrect salary value (%): Value must be non-negative', salary;
    END IF;

    result := (1.0+((bonus)::NUMERIC)/100.0) * 12 * salary;
    RETURN ROUND(result,2);
END;


$$LANGUAGE plpgsql;



SELECT calculate_reward(1000, 10) AS result;

-- Exception values
SELECT calculate_reward(-1000, 10) AS result;
SELECT calculate_reward(1000, -5) AS result;
SELECT calculate_reward(NULL, 10) AS result;
SELECT calculate_reward(1000, NULL) AS result;