DROP PROCEDURE insert_query;
DROP PROCEDURE update_query;


CREATE OR REPLACE PROCEDURE insert_query(id INTEGER, val INTEGER)
LANGUAGE plpgsql
AS
$$

BEGIN
    INSERT INTO MyTable (id, val) VALUES (id, val);
    RAISE NOTICE 'Record inserted successfully';
END;
$$;



CREATE OR REPLACE PROCEDURE update_query(aim_id INTEGER, new_val INTEGER)
LANGUAGE plpgsql
AS
$$

BEGIN
    UPDATE MyTable SET val=new_val where id=aim_id;
    RAISE NOTICE 'Record updated successfully';
END;
$$;



CREATE OR REPLACE PROCEDURE remove_query(aim_id INTEGER)
LANGUAGE plpgsql
AS
$$

BEGIN
    DELETE from MyTable where id=aim_id;
    RAISE NOTICE 'Record removed successfully';
END;
$$;


CALL insert_query(1, 1);
call update_query(1, 55555555);
CAll remove_query(1);