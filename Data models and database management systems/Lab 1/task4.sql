CREATE OR REPLACE FUNCTION generate_query(aim_id INTEGER)
RETURNS TEXT AS
$$
DECLARE
id INTEGER;
val INTEGER;
res TEXT;
BEGIN
    SELECT mt.id,mt.val INTO id,val FROM MyTable mt WHERE aim_id=mt.id Limit 1;

    if not found then
        RAISE NOTICE 'No record found';
        RETURN NULL;
    end if;
    
    res:=format('INSERT INTO MyTable (id, val) VALUES (%s, %s);', id, val);
    RAISE NOTICE '%',res;
    RETURN res;
    
END;
$$LANGUAGE plpgsql;

SELECT generate_query(4) AS result;