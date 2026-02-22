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
DROP FUNCTION IF EXISTS extra_tables_in_prod(text, text);
DROP FUNCTION IF EXISTS extra_procedures_in_prod(text, text);
DROP FUNCTION IF EXISTS extra_functions_in_prod(text, text);
DROP FUNCTION IF EXISTS extra_indexes_in_prod(text, text);
DROP FUNCTION IF EXISTS extra_packages_in_prod(text, text);
DROP FUNCTION IF EXISTS get_table_columns_ddl(text, text);
DROP FUNCTION IF EXISTS get_table_pk_ddl(text, text);
DROP FUNCTION IF EXISTS get_foreign_keys_ddl(text, text);
DROP FUNCTION IF EXISTS generate_sync_ddl(text, text);
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

CREATE OR REPLACE FUNCTION extra_tables_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(table_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT t.table_name::text
    FROM information_schema.tables t
    WHERE t.table_schema = prod_schema_name
      AND t.table_type = 'BASE TABLE'
      AND NOT EXISTS (
          SELECT 1
          FROM information_schema.tables t2   
          WHERE t2.table_schema = dev_schema_name
            AND t2.table_name = t.table_name   
            AND t2.table_type = 'BASE TABLE'
      )
    ORDER BY t.table_name;
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

CREATE OR REPLACE FUNCTION extra_procedures_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(proc_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH prod_proc AS (
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
        WHERE r.specific_schema = prod_schema_name
          AND r.routine_type = 'PROCEDURE'
    )
    SELECT prod.routine_name || '(' || prod.param_types || ')'::text
    FROM prod_proc prod
    WHERE NOT EXISTS (
        SELECT 1
        FROM information_schema.routines r2
        LEFT JOIN LATERAL (
            SELECT string_agg(p2.data_type::text, ',' ORDER BY p2.ordinal_position) AS pts
            FROM information_schema.parameters p2
            WHERE p2.specific_schema = r2.specific_schema
              AND p2.specific_name = r2.specific_name
        ) par ON true
        WHERE r2.specific_schema = dev_schema_name
          AND r2.routine_type = 'PROCEDURE'
          AND r2.routine_name = prod.routine_name
          AND COALESCE(par.pts, '') = prod.param_types
    );
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

CREATE OR REPLACE FUNCTION extra_functions_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(func_signature text) AS $$
BEGIN
    RETURN QUERY
    WITH prod_func AS (
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
        WHERE r.specific_schema = prod_schema_name
          AND r.routine_type = 'FUNCTION'
    )
    SELECT prod.routine_name || '(' || prod.param_types || ')'::text
    FROM prod_func prod
    WHERE NOT EXISTS (
        SELECT 1
        FROM information_schema.routines r2
        LEFT JOIN LATERAL (
            SELECT string_agg(p2.data_type::text, ',' ORDER BY p2.ordinal_position) AS pts
            FROM information_schema.parameters p2
            WHERE p2.specific_schema = r2.specific_schema
              AND p2.specific_name = r2.specific_name
        ) par ON true
        WHERE r2.specific_schema = dev_schema_name
          AND r2.routine_type = 'FUNCTION'
          AND r2.routine_name = prod.routine_name
          AND COALESCE(par.pts, '') = prod.param_types
    );
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

CREATE OR REPLACE FUNCTION extra_indexes_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(index_name text) AS $$
BEGIN
    RETURN QUERY
    SELECT i.indexname::text
    FROM pg_indexes i
    WHERE i.schemaname = prod_schema_name
      AND NOT EXISTS (
          SELECT 1 FROM pg_indexes i2
          WHERE i2.schemaname = dev_schema_name
            AND i2.indexname = i.indexname
      )
    ORDER BY i.indexname;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION missing_packages_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(package_name text) AS $$ BEGIN RETURN; END; $$ LANGUAGE plpgsql;
CREATE OR REPLACE FUNCTION mismatched_packages(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(package_name text) AS $$ BEGIN RETURN; END; $$ LANGUAGE plpgsql;
CREATE OR REPLACE FUNCTION extra_packages_in_prod(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(package_name text) AS $$ BEGIN RETURN; END; $$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION get_table_columns_ddl(schema_name text, tbl text)
RETURNS text AS $$
DECLARE
    col_line text;
    cols_text text := '';
    rec record;
BEGIN
    FOR rec IN
        SELECT column_name, data_type, is_nullable,
               coalesce(character_maximum_length::text, '') as char_len,
               coalesce(numeric_precision::text, '') as num_prec,
               coalesce(numeric_scale::text, '') as num_scale
        FROM information_schema.columns
        WHERE table_schema = schema_name AND table_name = tbl
        ORDER BY ordinal_position
    LOOP
        IF cols_text != '' THEN cols_text := cols_text || ', ' || E'\n  '; END IF;
        cols_text := cols_text || rec.column_name || ' ' || rec.data_type;
        IF rec.char_len != '' THEN
            cols_text := cols_text || '(' || rec.char_len || ')';
        END IF;
        IF rec.num_prec != '' AND rec.num_scale != '' THEN
            cols_text := cols_text || '(' || rec.num_prec || ',' || rec.num_scale || ')';
        END IF;
        IF rec.is_nullable = 'NO' THEN
            cols_text := cols_text || ' NOT NULL';
        END IF;
    END LOOP;
    RETURN cols_text;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_table_pk_ddl(schema_name text, tbl text)
RETURNS text AS $$
DECLARE
    pk_cols text;
BEGIN
    SELECT string_agg(kcu.column_name, ', ' ORDER BY kcu.ordinal_position)
    INTO pk_cols
    FROM information_schema.table_constraints tc
    JOIN information_schema.key_column_usage kcu
        ON tc.constraint_name = kcu.constraint_name
        AND tc.table_schema = kcu.constraint_schema
    WHERE tc.table_schema = schema_name
      AND tc.table_name = tbl
      AND tc.constraint_type = 'PRIMARY KEY';
    
    IF pk_cols IS NOT NULL THEN
        RETURN ', ' || E'\n  ' || 'CONSTRAINT ' || tbl || '_pkey PRIMARY KEY (' || pk_cols || ')';
    ELSE
        RETURN '';
    END IF;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_foreign_keys_ddl(schema_name text, tbl text)
RETURNS TABLE(fk_ddl text) AS $$
BEGIN
    RETURN QUERY
    SELECT
        'ALTER TABLE ' || quote_ident(schema_name) || '.' || quote_ident(tc.table_name) ||
        ' ADD CONSTRAINT ' || quote_ident(tc.constraint_name) ||
        ' FOREIGN KEY (' || string_agg(kcu.column_name, ', ' ORDER BY kcu.ordinal_position) || ')' ||
        ' REFERENCES ' || quote_ident(ccu.table_schema) || '.' || quote_ident(ccu.table_name) ||
        ' (' || string_agg(ccu.column_name, ', ' ORDER BY kcu.ordinal_position) || ');' AS fk_ddl
    FROM information_schema.table_constraints tc
    JOIN information_schema.key_column_usage kcu
        ON tc.constraint_name = kcu.constraint_name
        AND tc.table_schema = kcu.constraint_schema
    JOIN information_schema.constraint_column_usage ccu
        ON tc.constraint_name = ccu.constraint_name
        AND tc.table_schema = ccu.constraint_schema
    WHERE tc.table_schema = schema_name
      AND tc.table_name = tbl
      AND tc.constraint_type = 'FOREIGN KEY'
    GROUP BY tc.table_schema, tc.table_name, tc.constraint_name, ccu.table_schema, ccu.table_name;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_table_drop_ddl(schema_name text, tbl text)
RETURNS text AS $$
BEGIN
    RETURN 'DROP TABLE IF EXISTS ' || quote_ident(schema_name) || '.' || quote_ident(tbl) || ' CASCADE;';
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_routine_ddl(schema_name text, signature text)
RETURNS text AS $$
DECLARE
    routine_name text;
    param_types text;
    oid_val oid;
    ddl_text text;
BEGIN
    routine_name := split_part(signature, '(', 1);
    param_types := rtrim(split_part(signature, '(', 2), ')');
    
    SELECT p.oid INTO oid_val
    FROM pg_proc p
    JOIN pg_namespace n ON n.oid = p.pronamespace
    WHERE n.nspname = schema_name
      AND p.proname = routine_name
      AND pg_get_function_identity_arguments(p.oid) = param_types;
    
    IF oid_val IS NULL THEN
        RETURN '-- Routine ' || signature || ' not found in ' || schema_name;
    END IF;
    
    ddl_text := pg_get_functiondef(oid_val);
    RETURN ddl_text;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_index_ddl(schema_name text, index_name text)
RETURNS text AS $$
DECLARE
    def text;
BEGIN
    SELECT indexdef INTO def
    FROM pg_indexes
    WHERE schemaname = schema_name AND indexname = index_name;
    RETURN def || ';';
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION generate_sync_ddl(dev_schema_name text, prod_schema_name text)
RETURNS text AS $$
DECLARE
    ddl_script text := '';
    missing_tabs text[];
    mismatched_tabs text[];
    extra_tabs text[];
    all_drop_tabs text[];
    all_create_tabs text[];
    sorted_drop_tabs text[];
    sorted_create_tabs text[];
    tbl text;
    missing_procs text[];
    mismatched_procs text[];
    extra_procs text[];
    missing_funcs text[];
    mismatched_funcs text[];
    extra_funcs text[];
    missing_idxs text[];
    mismatched_idxs text[];
    extra_idxs text[];
    rec record;
    i integer;
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', dev_schema_name;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod_schema_name) THEN
        RAISE EXCEPTION 'Schema "%" does not exist', prod_schema_name;
    END IF;

    SELECT array_agg(table_name) INTO missing_tabs FROM missing_tables_in_prod(dev_schema_name, prod_schema_name);
    SELECT array_agg(table_name) INTO mismatched_tabs FROM mismatched_tables(dev_schema_name, prod_schema_name);
    SELECT array_agg(table_name) INTO extra_tabs FROM extra_tables_in_prod(dev_schema_name, prod_schema_name);

    SELECT array_agg(proc_signature) INTO missing_procs FROM missing_procedures_in_prod(dev_schema_name, prod_schema_name);
    SELECT array_agg(proc_signature) INTO mismatched_procs FROM mismatched_procedures(dev_schema_name, prod_schema_name);
    SELECT array_agg(proc_signature) INTO extra_procs FROM extra_procedures_in_prod(dev_schema_name, prod_schema_name);

    SELECT array_agg(func_signature) INTO missing_funcs FROM missing_functions_in_prod(dev_schema_name, prod_schema_name);
    SELECT array_agg(func_signature) INTO mismatched_funcs FROM mismatched_functions(dev_schema_name, prod_schema_name);
    SELECT array_agg(func_signature) INTO extra_funcs FROM extra_functions_in_prod(dev_schema_name, prod_schema_name);

    SELECT array_agg(index_name) INTO missing_idxs FROM missing_indexes_in_prod(dev_schema_name, prod_schema_name);
    SELECT array_agg(index_name) INTO mismatched_idxs FROM mismatched_indexes(dev_schema_name, prod_schema_name);
    SELECT array_agg(index_name) INTO extra_idxs FROM extra_indexes_in_prod(dev_schema_name, prod_schema_name);

    all_drop_tabs := array_cat(COALESCE(extra_tabs, '{}'), COALESCE(mismatched_tabs, '{}'));
    all_create_tabs := array_cat(COALESCE(missing_tabs, '{}'), COALESCE(mismatched_tabs, '{}'));

    IF array_length(all_drop_tabs, 1) > 0 THEN
        BEGIN
            SELECT array_agg(sorted_name) INTO sorted_drop_tabs
            FROM sort_tables_by_dependencies(all_drop_tabs, prod_schema_name);
            IF sorted_drop_tabs IS NOT NULL THEN
                sorted_drop_tabs := ARRAY(
                    SELECT sorted_drop_tabs[i]
                    FROM generate_series(array_length(sorted_drop_tabs,1), 1, -1) i
                );
            END IF;
        EXCEPTION WHEN OTHERS THEN
            RAISE NOTICE 'Circular dependency among tables to drop, using alphabetical order: %', SQLERRM;
            SELECT array_agg(t ORDER BY t DESC) INTO sorted_drop_tabs FROM unnest(all_drop_tabs) t;
        END;
    END IF;

    IF array_length(all_create_tabs, 1) > 0 THEN
        BEGIN
            SELECT array_agg(sorted_name) INTO sorted_create_tabs
            FROM sort_tables_by_dependencies(all_create_tabs, dev_schema_name);
        EXCEPTION WHEN OTHERS THEN
            RAISE NOTICE 'Circular dependency among tables to create, using alphabetical order: %', SQLERRM;
            SELECT array_agg(t ORDER BY t) INTO sorted_create_tabs FROM unnest(all_create_tabs) t;
        END;
    END IF;

    ddl_script := '-- Synchronization script from ' || dev_schema_name || ' to ' || prod_schema_name || E'\n';
    ddl_script := ddl_script || 'BEGIN;' || E'\n\n';

    IF extra_idxs IS NOT NULL OR mismatched_idxs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Dropping extra and mismatched indexes' || E'\n';
        IF extra_idxs IS NOT NULL THEN
            FOR i IN 1..array_length(extra_idxs, 1) LOOP
                ddl_script := ddl_script || 'DROP INDEX IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(extra_idxs[i]) || ';' || E'\n';
            END LOOP;
        END IF;
        IF mismatched_idxs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_idxs, 1) LOOP
                ddl_script := ddl_script || 'DROP INDEX IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(mismatched_idxs[i]) || ';' || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF extra_procs IS NOT NULL OR mismatched_procs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Dropping extra and mismatched procedures' || E'\n';
        IF extra_procs IS NOT NULL THEN
            FOR i IN 1..array_length(extra_procs, 1) LOOP
                ddl_script := ddl_script || 'DROP PROCEDURE IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(split_part(extra_procs[i],'(',1)) || '; -- ' || extra_procs[i] || E'\n';
            END LOOP;
        END IF;
        IF mismatched_procs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_procs, 1) LOOP
                ddl_script := ddl_script || 'DROP PROCEDURE IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(split_part(mismatched_procs[i],'(',1)) || '; -- ' || mismatched_procs[i] || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF extra_funcs IS NOT NULL OR mismatched_funcs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Dropping extra and mismatched functions' || E'\n';
        IF extra_funcs IS NOT NULL THEN
            FOR i IN 1..array_length(extra_funcs, 1) LOOP
                ddl_script := ddl_script || 'DROP FUNCTION IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(split_part(extra_funcs[i],'(',1)) || '; -- ' || extra_funcs[i] || E'\n';
            END LOOP;
        END IF;
        IF mismatched_funcs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_funcs, 1) LOOP
                ddl_script := ddl_script || 'DROP FUNCTION IF EXISTS ' || quote_ident(prod_schema_name) || '.' || quote_ident(split_part(mismatched_funcs[i],'(',1)) || '; -- ' || mismatched_funcs[i] || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF sorted_drop_tabs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Dropping extra and mismatched tables (with dependencies)' || E'\n';
        FOREACH tbl IN ARRAY sorted_drop_tabs LOOP
            ddl_script := ddl_script || get_table_drop_ddl(prod_schema_name, tbl) || E'\n';
        END LOOP;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF sorted_create_tabs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Creating missing and replaced tables' || E'\n';
        FOREACH tbl IN ARRAY sorted_create_tabs LOOP
            ddl_script := ddl_script || 'CREATE TABLE ' || quote_ident(prod_schema_name) || '.' || quote_ident(tbl) || ' (' || E'\n  ';
            ddl_script := ddl_script || get_table_columns_ddl(dev_schema_name, tbl);
            ddl_script := ddl_script || get_table_pk_ddl(dev_schema_name, tbl);
            ddl_script := ddl_script || E'\n' || ');' || E'\n\n';
        END LOOP;
    END IF;

    IF sorted_create_tabs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Adding foreign keys' || E'\n';
        FOREACH tbl IN ARRAY sorted_create_tabs LOOP
            FOR rec IN SELECT fk_ddl FROM get_foreign_keys_ddl(dev_schema_name, tbl) LOOP
                ddl_script := ddl_script || replace(rec.fk_ddl, quote_ident(dev_schema_name) || '.', quote_ident(prod_schema_name) || '.') || E'\n';
            END LOOP;
        END LOOP;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF missing_procs IS NOT NULL OR mismatched_procs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Creating/replacing procedures' || E'\n';
        IF missing_procs IS NOT NULL THEN
            FOR i IN 1..array_length(missing_procs, 1) LOOP
                ddl_script := ddl_script || get_routine_ddl(dev_schema_name, missing_procs[i]) || E'\n';
            END LOOP;
        END IF;
        IF mismatched_procs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_procs, 1) LOOP
                ddl_script := ddl_script || get_routine_ddl(dev_schema_name, mismatched_procs[i]) || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF missing_funcs IS NOT NULL OR mismatched_funcs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Creating/replacing functions' || E'\n';
        IF missing_funcs IS NOT NULL THEN
            FOR i IN 1..array_length(missing_funcs, 1) LOOP
                ddl_script := ddl_script || get_routine_ddl(dev_schema_name, missing_funcs[i]) || E'\n';
            END LOOP;
        END IF;
        IF mismatched_funcs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_funcs, 1) LOOP
                ddl_script := ddl_script || get_routine_ddl(dev_schema_name, mismatched_funcs[i]) || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    IF missing_idxs IS NOT NULL OR mismatched_idxs IS NOT NULL THEN
        ddl_script := ddl_script || '-- Creating indexes' || E'\n';
        IF missing_idxs IS NOT NULL THEN
            FOR i IN 1..array_length(missing_idxs, 1) LOOP
                ddl_script := ddl_script || replace(get_index_ddl(dev_schema_name, missing_idxs[i]), quote_ident(dev_schema_name) || '.', quote_ident(prod_schema_name) || '.') || E'\n';
            END LOOP;
        END IF;
        IF mismatched_idxs IS NOT NULL THEN
            FOR i IN 1..array_length(mismatched_idxs, 1) LOOP
                ddl_script := ddl_script || replace(get_index_ddl(dev_schema_name, mismatched_idxs[i]), quote_ident(dev_schema_name) || '.', quote_ident(prod_schema_name) || '.') || E'\n';
            END LOOP;
        END IF;
        ddl_script := ddl_script || E'\n';
    END IF;

    ddl_script := ddl_script || 'COMMIT;' || E'\n';
    RETURN ddl_script;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION compare_schemas(dev_schema_name text, prod_schema_name text)
RETURNS TABLE(object_name text, object_type text, status text) AS $$
DECLARE
    tbls text[];
    sorted_tbls text[];
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
    SELECT table_name, 'TABLE', 'missing' FROM missing_tables_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT table_name, 'TABLE', 'mismatch' FROM mismatched_tables(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT proc_signature, 'PROCEDURE', 'missing' FROM missing_procedures_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT proc_signature, 'PROCEDURE', 'mismatch' FROM mismatched_procedures(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT func_signature, 'FUNCTION', 'missing' FROM missing_functions_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT func_signature, 'FUNCTION', 'mismatch' FROM mismatched_functions(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT index_name, 'INDEX', 'missing' FROM missing_indexes_in_prod(dev_schema_name, prod_schema_name)
    UNION ALL
    SELECT index_name, 'INDEX', 'mismatch' FROM mismatched_indexes(dev_schema_name, prod_schema_name);

    IF NOT EXISTS (SELECT 1 FROM tmp_results) THEN
        RETURN;
    END IF;

    SELECT array_agg(tmp_results.object_name) INTO tbls FROM tmp_results WHERE tmp_results.object_type = 'TABLE';

    IF tbls IS NOT NULL THEN
        BEGIN
            SELECT array_agg(sorted_name) INTO sorted_tbls FROM sort_tables_by_dependencies(tbls, dev_schema_name);
        EXCEPTION WHEN OTHERS THEN
            RAISE NOTICE 'Circular dependency among tables: %', SQLERRM;
            SELECT array_agg(tmp_results.object_name) INTO sorted_tbls FROM tmp_results WHERE tmp_results.object_type = 'TABLE' ORDER BY tmp_results.object_name;
            circular_deps := true;
        END;
    END IF;

    IF sorted_tbls IS NOT NULL THEN
        FOR i IN 1..array_length(sorted_tbls, 1) LOOP
            SELECT tmp_results.object_name, tmp_results.object_type, tmp_results.status 
            INTO object_name, object_type, status
            FROM tmp_results
            WHERE tmp_results.object_name = sorted_tbls[i] AND tmp_results.object_type = 'TABLE';
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
BEGIN RAISE NOTICE 'Hello from dev'; END; $$;
CREATE OR REPLACE PROCEDURE prod.test_proc() LANGUAGE plpgsql AS $$
BEGIN RAISE NOTICE 'Hello from prod'; END; $$;

CREATE OR REPLACE FUNCTION dev.add(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN RETURN a + b; END; $$;
CREATE OR REPLACE FUNCTION prod.add(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN RETURN a + b + 1; END; $$;
CREATE OR REPLACE FUNCTION dev.multiply(a INTEGER, b INTEGER) RETURNS INTEGER LANGUAGE plpgsql AS $$
BEGIN RETURN a * b; END; $$;

CREATE INDEX IF NOT EXISTS idx_users_name ON dev.users(name);
CREATE INDEX IF NOT EXISTS idx_users_name ON prod.users(name);
CREATE INDEX IF NOT EXISTS idx_orders_amount ON dev.orders(amount);

CREATE INDEX IF NOT EXISTS idx_products_price ON dev.products(price);
CREATE INDEX IF NOT EXISTS idx_products_price ON prod.products(price) WHERE price > 100;

SELECT '=== Comparison result ===' as info;
SELECT * FROM compare_schemas('dev', 'prod');

SELECT '=== Generated DDL script ===' as info;
SELECT generate_sync_ddl('dev', 'prod');