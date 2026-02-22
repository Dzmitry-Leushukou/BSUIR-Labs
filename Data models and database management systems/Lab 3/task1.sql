DROP FUNCTION IF EXISTS compare_schemas(text, text);
DROP FUNCTION IF EXISTS missing_tables_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_tables(text, text);
DROP FUNCTION IF EXISTS sort_tables_by_dependencies(text[], text);
DROP SCHEMA IF EXISTS dev CASCADE;
DROP SCHEMA IF EXISTS prod CASCADE;

CREATE OR REPLACE FUNCTION missing_tables_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(table_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT t.table_name::text
    FROM information_schema.tables t
    WHERE t.table_schema = dev_schema_name
      AND t.table_type = 'BASE TABLE'
      AND NOT EXISTS (
          SELECT 1
          FROM information_schema.tables t2   
          WHERE t2.table_schema = prod_schema_name
            AND t2.table_name = t.table_name   
            AND t2.table_type = 'BASE TABLE'
      )
    ORDER BY t.table_name;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION mismatched_tables(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(table_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT dev.table_name::text
    FROM (
        SELECT c.table_name,
               array_agg(c.column_name || ':' || c.data_type || ':' || c.is_nullable ORDER BY c.column_name) AS cols
        FROM information_schema.columns c
        WHERE c.table_schema = dev_schema_name
        GROUP BY c.table_name
    ) dev
    JOIN (
        SELECT c.table_name,
               array_agg(c.column_name || ':' || c.data_type || ':' || c.is_nullable ORDER BY c.column_name) AS cols
        FROM information_schema.columns c
        WHERE c.table_schema = prod_schema_name
        GROUP BY c.table_name
    ) prod ON dev.table_name = prod.table_name
    WHERE dev.cols <> prod.cols
    ORDER BY dev.table_name;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION sort_tables_by_dependencies(
    table_names text[],
    schema_name text
)
RETURNS TABLE(sorted_name text) AS $$
DECLARE
    deps TEXT[];                     
    subset TEXT[];                    
    indegree INTEGER[];               
    order_list TEXT[];                
    i INTEGER;
    j INTEGER;
    k INTEGER;
    v_child TEXT;                     
    v_parent TEXT;
    tbl TEXT;
    found BOOLEAN;
    candidates TEXT[];
    min_tbl TEXT;
BEGIN
    WITH fk AS (
        SELECT
            kcu.table_name AS child,
            ccu.table_name AS parent
        FROM information_schema.table_constraints tc
        JOIN information_schema.key_column_usage kcu 
            ON tc.constraint_name = kcu.constraint_name 
            AND tc.table_schema = kcu.constraint_schema
        JOIN information_schema.constraint_column_usage ccu 
            ON tc.constraint_name = ccu.constraint_name 
            AND tc.table_schema = ccu.constraint_schema
        WHERE tc.constraint_type = 'FOREIGN KEY'
          AND tc.table_schema = schema_name
          AND ccu.table_schema = schema_name
        GROUP BY child, parent
    )
    SELECT array_agg(child || ':' || parent)
    INTO deps
    FROM fk;

    IF deps IS NULL THEN
        deps := '{}';
    END IF;

    subset := table_names;

    indegree := array_fill(0, array[array_length(subset, 1)]);

    FOR i IN 1..array_length(deps, 1) LOOP
        v_child := split_part(deps[i], ':', 1);
        v_parent := split_part(deps[i], ':', 2);

        FOR j IN 1..array_length(subset, 1) LOOP
            IF subset[j] = v_child THEN
                FOR k IN 1..array_length(subset, 1) LOOP
                    IF subset[k] = v_parent THEN
                        indegree[j] := indegree[j] + 1; 
                        EXIT;
                    END IF;
                END LOOP;
                EXIT;
            END IF;
        END LOOP;
    END LOOP;

    order_list := '{}';
    LOOP
        found := false;
        candidates := '{}';

        FOR i IN 1..array_length(subset, 1) LOOP
            IF indegree[i] = 0 AND NOT (subset[i] = ANY(order_list)) THEN
                candidates := candidates || subset[i];
            END IF;
        END LOOP;

        IF array_length(candidates, 1) > 0 THEN
            SELECT min(c) INTO min_tbl FROM unnest(candidates) c;
            order_list := order_list || min_tbl;
            found := true;

            FOR i IN 1..array_length(subset, 1) LOOP
                IF subset[i] = min_tbl THEN
                    FOR j IN 1..array_length(deps, 1) LOOP
                        v_child := split_part(deps[j], ':', 1);
                        v_parent := split_part(deps[j], ':', 2);
                        IF v_parent = min_tbl THEN
                            FOR k IN 1..array_length(subset, 1) LOOP
                                IF subset[k] = v_child THEN
                                    indegree[k] := indegree[k] - 1;
                                    EXIT;
                                END IF;
                            END LOOP;
                        END IF;
                    END LOOP;
                    EXIT;
                END IF;
            END LOOP;
        END IF;

        IF NOT found THEN
            EXIT;
        END IF;
    END LOOP;

    IF array_length(order_list, 1) <> array_length(subset, 1) THEN
        RAISE EXCEPTION 'Circular dependency detected among tables: %', 
            array_to_string(
                (SELECT array_agg(t) FROM unnest(subset) t WHERE NOT (t = ANY(order_list))),
                ', '
            );
    END IF;

    FOR i IN 1..array_length(order_list, 1) LOOP
        sorted_name := order_list[i];
        RETURN NEXT;
    END LOOP;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION compare_schemas(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(table_name text, status text) AS $$
DECLARE
    tbls text[];
    sorted_tbls text[];
    i int;
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', dev_schema_name;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', prod_schema_name;
    END IF;

    CREATE TEMP TABLE tmp_result (table_name text, status text) ON COMMIT DROP;

    INSERT INTO tmp_result (table_name, status)
    SELECT missing.table_name, 'missing'
    FROM missing_tables_in_prod(dev_schema_name, prod_schema_name) missing
    UNION ALL
    SELECT mismatched.table_name, 'mismatch'
    FROM mismatched_tables(dev_schema_name, prod_schema_name) mismatched;

    IF NOT EXISTS (SELECT 1 FROM tmp_result) THEN
        RETURN;
    END IF;

    SELECT array_agg(tmp_result.table_name) INTO tbls FROM tmp_result;

    SELECT array_agg(sorted_name) INTO sorted_tbls
    FROM sort_tables_by_dependencies(tbls, dev_schema_name);

    FOR i IN 1..array_length(sorted_tbls, 1) LOOP
        SELECT tmp_result.status INTO status FROM tmp_result WHERE tmp_result.table_name = sorted_tbls[i];
        table_name := sorted_tbls[i];
        RETURN NEXT;
    END LOOP;
END;
$$ LANGUAGE plpgsql;

CREATE SCHEMA IF NOT EXISTS dev;
CREATE SCHEMA IF NOT EXISTS prod;

CREATE TABLE IF NOT EXISTS dev.users (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    email TEXT UNIQUE
);

CREATE TABLE IF NOT EXISTS dev.logs (
    id SERIAL PRIMARY KEY,
    event TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS dev.products (
    id SERIAL PRIMARY KEY,
    title TEXT,
    price NUMERIC
);

CREATE TABLE IF NOT EXISTS prod.users (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    email TEXT UNIQUE,
    phone TEXT  
);

CREATE TABLE IF NOT EXISTS prod.payments (
    id SERIAL PRIMARY KEY,
    user_id INTEGER,
    amount NUMERIC,
    paid_at TIMESTAMP
);

CREATE TABLE IF NOT EXISTS prod.products (
    id SERIAL PRIMARY KEY,
    title TEXT,
    price NUMERIC,
    stock INTEGER   
);

CREATE TABLE IF NOT EXISTS dev.orders (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES dev.users(id),
    amount NUMERIC
);

SELECT * FROM compare_schemas('dev', 'prod');