DROP FUNCTION IF EXISTS compare_schemas(text, text);
DROP FUNCTION IF EXISTS missing_tables_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_tables(text, text);
DROP FUNCTION IF EXISTS sort_tables_by_dependencies(text[], text);
DROP FUNCTION IF EXISTS missing_procedures_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_procedures(text, text);
DROP FUNCTION IF EXISTS missing_functions_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_functions(text, text);
DROP FUNCTION IF EXISTS missing_indexes_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_indexes(text, text);
DROP FUNCTION IF EXISTS missing_packages_in_prod(text, text);
DROP FUNCTION IF EXISTS mismatched_packages(text, text);
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

CREATE OR REPLACE FUNCTION missing_procedures_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(proc_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH dev_proc AS (
        SELECT 
            r.routine_name,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = dev_schema_name
          AND r.routine_type = 'PROCEDURE'
    )
    SELECT dev.routine_name || '(' || dev.param_types || ')'::text
    FROM dev_proc dev
    WHERE NOT EXISTS (
        SELECT 1
        FROM information_schema.routines r2
        LEFT JOIN LATERAL (
            SELECT string_agg(p2.data_type::text, ',' ORDER BY p2.ordinal_position) AS pts
            FROM information_schema.parameters p2
            WHERE p2.specific_schema = r2.specific_schema
              AND p2.specific_name = r2.specific_name
        ) par ON true
        WHERE r2.specific_schema = prod_schema_name
          AND r2.routine_type = 'PROCEDURE'
          AND r2.routine_name = dev.routine_name
          AND COALESCE(par.pts, '') = dev.param_types
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION mismatched_procedures(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(proc_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH dev_proc AS (
        SELECT 
            r.routine_name,
            r.routine_definition,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = dev_schema_name
          AND r.routine_type = 'PROCEDURE'
    ),
    prod_proc AS (
        SELECT 
            r.routine_name,
            r.routine_definition,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = prod_schema_name
          AND r.routine_type = 'PROCEDURE'
    )
    SELECT dev.routine_name || '(' || dev.param_types || ')'::text
    FROM dev_proc dev
    JOIN prod_proc prod 
        ON dev.routine_name = prod.routine_name 
        AND dev.param_types = prod.param_types
    WHERE dev.routine_definition IS DISTINCT FROM prod.routine_definition;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION missing_functions_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(func_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH dev_func AS (
        SELECT 
            r.routine_name,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = dev_schema_name
          AND r.routine_type = 'FUNCTION'
    )
    SELECT dev.routine_name || '(' || dev.param_types || ')'::text
    FROM dev_func dev
    WHERE NOT EXISTS (
        SELECT 1
        FROM information_schema.routines r2
        LEFT JOIN LATERAL (
            SELECT string_agg(p2.data_type::text, ',' ORDER BY p2.ordinal_position) AS pts
            FROM information_schema.parameters p2
            WHERE p2.specific_schema = r2.specific_schema
              AND p2.specific_name = r2.specific_name
        ) par ON true
        WHERE r2.specific_schema = prod_schema_name
          AND r2.routine_type = 'FUNCTION'
          AND r2.routine_name = dev.routine_name
          AND COALESCE(par.pts, '') = dev.param_types
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION mismatched_functions(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(func_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH dev_func AS (
        SELECT 
            r.routine_name,
            r.routine_definition,
            r.data_type AS return_type,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = dev_schema_name
          AND r.routine_type = 'FUNCTION'
    ),
    prod_func AS (
        SELECT 
            r.routine_name,
            r.routine_definition,
            r.data_type AS return_type,
            COALESCE(
                (SELECT string_agg(p.data_type::text, ',' ORDER BY p.ordinal_position)
                 FROM information_schema.parameters p
                 WHERE p.specific_schema = r.specific_schema
                   AND p.specific_name = r.specific_name
                ), ''
            ) AS param_types
        FROM information_schema.routines r
        WHERE r.specific_schema = prod_schema_name
          AND r.routine_type = 'FUNCTION'
    )
    SELECT dev.routine_name || '(' || dev.param_types || ')'::text
    FROM dev_func dev
    JOIN prod_func prod 
        ON dev.routine_name = prod.routine_name 
        AND dev.param_types = prod.param_types
    WHERE dev.routine_definition IS DISTINCT FROM prod.routine_definition
       OR dev.return_type IS DISTINCT FROM prod.return_type;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION missing_indexes_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(index_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT i.indexname::text
    FROM pg_indexes i
    WHERE i.schemaname = dev_schema_name
      AND NOT EXISTS (
          SELECT 1 FROM pg_indexes i2
          WHERE i2.schemaname = prod_schema_name
            AND i2.indexname = i.indexname
      )
    ORDER BY i.indexname;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION mismatched_indexes(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(index_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT dev.indexname::text
    FROM pg_indexes dev
    JOIN pg_indexes prod 
        ON dev.indexname = prod.indexname
        AND prod.schemaname = prod_schema_name
    WHERE dev.schemaname = dev_schema_name
      AND dev.indexdef IS DISTINCT FROM prod.indexdef
    ORDER BY dev.indexname;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION missing_packages_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(package_name text) AS $$
BEGIN
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION mismatched_packages(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(package_name text) AS $$
BEGIN
    RETURN;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION compare_schemas(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(object_name text, object_type text, status text) AS $$
DECLARE
    tbls text[];
    sorted_tbls text[];
    tbl_rec record;
    circular_deps boolean := false;
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', dev_schema_name;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', prod_schema_name;
    END IF;

    CREATE TEMP TABLE tmp_results (object_name text, object_type text, status text) ON COMMIT DROP;

    INSERT INTO tmp_results (object_name, object_type, status)
    SELECT table_name, 'TABLE', 'missing'
    FROM missing_tables_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT table_name, 'TABLE', 'mismatch'
    FROM mismatched_tables(dev_schema_name, prod_schema_name);

    INSERT INTO tmp_results (object_name, object_type, status)
    SELECT proc_signature, 'PROCEDURE', 'missing'
    FROM missing_procedures_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT proc_signature, 'PROCEDURE', 'mismatch'
    FROM mismatched_procedures(dev_schema_name, prod_schema_name);

    INSERT INTO tmp_results (object_name, object_type, status)
    SELECT func_signature, 'FUNCTION', 'missing'
    FROM missing_functions_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT func_signature, 'FUNCTION', 'mismatch'
    FROM mismatched_functions(dev_schema_name, prod_schema_name);

    INSERT INTO tmp_results (object_name, object_type, status)
    SELECT index_name, 'INDEX', 'missing'
    FROM missing_indexes_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT index_name, 'INDEX', 'mismatch'
    FROM mismatched_indexes(dev_schema_name, prod_schema_name);

    INSERT INTO tmp_results (object_name, object_type, status)
    SELECT package_name, 'PACKAGE', 'missing'
    FROM missing_packages_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT package_name, 'PACKAGE', 'mismatch'
    FROM mismatched_packages(dev_schema_name, prod_schema_name);

    IF NOT EXISTS (SELECT 1 FROM tmp_results) THEN
        RETURN;
    END IF;

    SELECT array_agg(tmp_results.object_name) INTO tbls
    FROM tmp_results
    WHERE tmp_results.object_type = 'TABLE';

    IF tbls IS NOT NULL THEN
        BEGIN
            SELECT array_agg(sorted_name) INTO sorted_tbls
            FROM sort_tables_by_dependencies(tbls, dev_schema_name);
        EXCEPTION WHEN OTHERS THEN
            RAISE NOTICE 'Circular dependency detected among tables: %', SQLERRM;
            SELECT array_agg(tmp_results.object_name) INTO sorted_tbls
            FROM tmp_results
            WHERE tmp_results.object_type = 'TABLE'
            ORDER BY tmp_results.object_name;
            circular_deps := true;
        END;
    END IF;

    IF sorted_tbls IS NOT NULL THEN
        FOR i IN 1..array_length(sorted_tbls, 1) LOOP
            SELECT tmp_results.object_name, tmp_results.object_type, tmp_results.status 
            INTO object_name, object_type, status
            FROM tmp_results
            WHERE tmp_results.object_name = sorted_tbls[i] 
              AND tmp_results.object_type = 'TABLE';
            RETURN NEXT;
        END LOOP;
    END IF;

    FOR object_name, object_type, status IN
        SELECT tmp_results.object_name, tmp_results.object_type, tmp_results.status
        FROM tmp_results
        WHERE tmp_results.object_type != 'TABLE'
        ORDER BY tmp_results.object_type, tmp_results.object_name
    LOOP
        RETURN NEXT;
    END LOOP;

    IF circular_deps THEN
        object_name := 'Circular dependency detected among tables, displayed in alphabetical order';
        object_type := 'WARNING';
        status := '';
        RETURN NEXT;
    END IF;
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
CREATE TABLE IF NOT EXISTS dev.orders (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES dev.users(id),
    amount NUMERIC
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


CREATE OR REPLACE PROCEDURE dev.test_proc() LANGUAGE plpgsql AS $$
BEGIN
    RAISE NOTICE 'Hello from dev';
END;
$$;
CREATE OR REPLACE PROCEDURE prod.test_proc() LANGUAGE plpgsql AS $$
BEGIN
    RAISE NOTICE 'Hello from prod';  
END;
$$;

CREATE OR REPLACE FUNCTION dev.add(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN
    RETURN a + b;
END;
$$;
CREATE OR REPLACE FUNCTION prod.add(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN
    RETURN a + b + 1; 
END;
$$;

CREATE OR REPLACE FUNCTION dev.multiply(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN
    RETURN a * b;
END;
$$;

CREATE INDEX IF NOT EXISTS idx_users_name ON dev.users(name);
CREATE INDEX IF NOT EXISTS idx_users_name ON prod.users(name);  

CREATE INDEX IF NOT EXISTS idx_orders_amount ON dev.orders(amount);

CREATE INDEX IF NOT EXISTS idx_products_price ON dev.products(price);
CREATE INDEX IF NOT EXISTS idx_products_price ON prod.products(price) WHERE price > 100; 

SELECT * FROM compare_schemas('dev', 'prod');