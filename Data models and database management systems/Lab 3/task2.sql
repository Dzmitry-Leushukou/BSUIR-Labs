DROP FUNCTION IF EXISTS compare_schemas_extended(TEXT, TEXT);

CREATE OR REPLACE FUNCTION compare_schemas_extended(dev TEXT, prod TEXT) 
RETURNS TABLE(
    obj_type TEXT,
    obj_name TEXT,
    issue_type TEXT,
    issue_details TEXT,
    creation_order INT
) 
LANGUAGE plpgsql AS $$
DECLARE
    cycle_found BOOL;
    cycle_info TEXT;
BEGIN
    -- Check schema existence
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev) THEN
        RAISE EXCEPTION 'Schema % does not exist', dev;
    END IF;
    
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod) THEN
        RAISE EXCEPTION 'Schema % does not exist', prod;
    END IF;

    -- 1. Check for circular dependencies in tables
    WITH RECURSIVE fk_cycle AS (
        SELECT conrelid::regclass AS child, confrelid::regclass AS parent,
               ARRAY[conrelid::regclass] AS path
        FROM pg_constraint 
        WHERE contype = 'f' 
          AND connamespace IN (dev::regnamespace, prod::regnamespace)
        UNION ALL
        SELECT c.conrelid::regclass, c.confrelid::regclass, fc.path || c.conrelid::regclass
        FROM pg_constraint c
        JOIN fk_cycle fc ON c.confrelid = fc.child
        WHERE c.conrelid <> ALL(fc.path)
    )
    SELECT EXISTS(SELECT 1 FROM fk_cycle WHERE parent = ANY(path)), 
           STRING_AGG(DISTINCT child::TEXT, ', ')
    INTO cycle_found, cycle_info
    FROM fk_cycle 
    WHERE parent = ANY(path);

    IF cycle_found THEN
        obj_type := 'CYCLE';
        obj_name := '';
        issue_type := 'CYCLIC_DEPENDENCY';
        issue_details := 'Circular FK dependencies: ' || cycle_info;
        creation_order := 0;
        RETURN NEXT;
        RETURN;
    END IF;

    -- 2. Get table dependencies for topological sort
    CREATE TEMP TABLE table_deps ON COMMIT DROP AS
    WITH RECURSIVE topo AS (
        SELECT c.oid, c.relname, 1 AS lvl
        FROM pg_class c
        WHERE c.relnamespace = prod::regnamespace 
          AND c.relkind = 'r'
          AND NOT EXISTS (
              SELECT 1 FROM pg_constraint fk 
              WHERE fk.conrelid = c.oid AND fk.contype = 'f'
          )
        UNION
        SELECT c.oid, c.relname, t.lvl + 1
        FROM topo t
        JOIN pg_constraint fk ON fk.confrelid = t.oid
        JOIN pg_class c ON c.oid = fk.conrelid
    ) SELECT relname, ROW_NUMBER() OVER (ORDER BY MIN(lvl), relname) AS rn
      FROM topo 
      GROUP BY relname;

    -- 3. Compare different types of objects
    RETURN QUERY
    WITH all_results AS (
        -- 3.1 Tables
        SELECT 
            'TABLE'::TEXT as result_obj_type,
            d.relname::TEXT as result_obj_name,
            CASE WHEN p.relname IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END as result_issue_type,
            CASE WHEN p.relname IS NULL THEN 'Table missing in prod'
                 ELSE 'Column structure differs' END as result_issue_details,
            COALESCE(td.rn, 999999)::INT as result_creation_order,
            1 as type_order
        FROM (
            SELECT c.relname, 
                   STRING_AGG(a.attname || ':' || 
                             format_type(a.atttypid, a.atttypmod) || ':' ||
                             CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE 'NULL' END, 
                             '|' ORDER BY a.attnum) AS sig
            FROM pg_class c
            JOIN pg_attribute a ON a.attrelid = c.oid
            WHERE c.relnamespace = dev::regnamespace 
              AND c.relkind = 'r'
              AND a.attnum > 0 
              AND NOT a.attisdropped
            GROUP BY c.relname
        ) d
        LEFT JOIN (
            SELECT c.relname, 
                   STRING_AGG(a.attname || ':' || 
                             format_type(a.atttypid, a.atttypmod) || ':' ||
                             CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE 'NULL' END, 
                             '|' ORDER BY a.attnum) AS sig
            FROM pg_class c
            JOIN pg_attribute a ON a.attrelid = c.oid
            WHERE c.relnamespace = prod::regnamespace 
              AND c.relkind = 'r'
              AND a.attnum > 0 
              AND NOT a.attisdropped
            GROUP BY c.relname
        ) p ON d.relname = p.relname
        LEFT JOIN table_deps td ON d.relname = td.relname
        WHERE p.relname IS NULL OR d.sig != p.sig
        
        UNION ALL
        
        -- 3.2 Indexes
        SELECT 
            'INDEX'::TEXT,
            (d.table_name || '.' || d.index_name)::TEXT,
            CASE WHEN p.index_name IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END,
            CASE WHEN p.index_name IS NULL THEN 'Index missing in prod'
                 ELSE 'Index definition differs' END,
            COALESCE(td.rn, 999999)::INT,
            2
        FROM (
            SELECT c.relname AS table_name,
                   i.relname AS index_name,
                   pg_get_indexdef(i.oid) AS index_def
            FROM pg_index idx
            JOIN pg_class i ON i.oid = idx.indexrelid
            JOIN pg_class c ON c.oid = idx.indrelid
            WHERE i.relnamespace = dev::regnamespace
              AND i.relkind = 'i'
        ) d
        LEFT JOIN (
            SELECT c.relname AS table_name,
                   i.relname AS index_name,
                   pg_get_indexdef(i.oid) AS index_def
            FROM pg_index idx
            JOIN pg_class i ON i.oid = idx.indexrelid
            JOIN pg_class c ON c.oid = idx.indrelid
            WHERE i.relnamespace = prod::regnamespace
              AND i.relkind = 'i'
        ) p ON d.table_name = p.table_name 
             AND d.index_name = p.index_name
        LEFT JOIN table_deps td ON d.table_name = td.relname
        WHERE p.index_name IS NULL OR d.index_def != p.index_def
        
        UNION ALL
        
        -- 3.3 Functions and Procedures
        SELECT 
            d.func_type,
            (d.func_name || '(' || d.args || ')')::TEXT,
            CASE WHEN p.func_name IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END,
            CASE WHEN p.func_name IS NULL THEN 'Function/Procedure missing in prod'
                 ELSE 'Function/Procedure definition differs' END,
            999999::INT,
            6
        FROM (
            SELECT p.proname AS func_name,
                   pg_get_function_arguments(p.oid) AS args,
                   pg_get_functiondef(p.oid) AS func_def,
                   CASE p.prokind 
                       WHEN 'f' THEN 'FUNCTION'
                       WHEN 'p' THEN 'PROCEDURE'
                       ELSE 'ROUTINE' 
                   END AS func_type
            FROM pg_proc p
            WHERE p.pronamespace = dev::regnamespace
        ) d
        LEFT JOIN (
            SELECT p.proname AS func_name,
                   pg_get_function_arguments(p.oid) AS args,
                   pg_get_functiondef(p.oid) AS func_def,
                   CASE p.prokind 
                       WHEN 'f' THEN 'FUNCTION'
                       WHEN 'p' THEN 'PROCEDURE'
                       ELSE 'ROUTINE' 
                   END AS func_type
            FROM pg_proc p
            WHERE p.pronamespace = prod::regnamespace
        ) p ON d.func_name = p.func_name 
            AND d.args = p.args
        WHERE p.func_name IS NULL OR d.func_def != p.func_def
        
        UNION ALL
        
        -- 3.4 Views
        SELECT 
            'VIEW'::TEXT,
            d.view_name::TEXT,
            CASE WHEN p.view_name IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END,
            CASE WHEN p.view_name IS NULL THEN 'View missing in prod'
                 ELSE 'View definition differs' END,
            999999::INT,
            4
        FROM (
            SELECT c.relname AS view_name,
                   pg_get_viewdef(c.oid) AS view_def
            FROM pg_class c
            WHERE c.relnamespace = dev::regnamespace
              AND c.relkind = 'v'
        ) d
        LEFT JOIN (
            SELECT c.relname AS view_name,
                   pg_get_viewdef(c.oid) AS view_def
            FROM pg_class c
            WHERE c.relnamespace = prod::regnamespace
              AND c.relkind = 'v'
        ) p ON d.view_name = p.view_name
        WHERE p.view_name IS NULL OR d.view_def != p.view_def
        
        UNION ALL
        
        -- 3.5 Sequences
        SELECT 
            'SEQUENCE'::TEXT,
            d.seq_name::TEXT,
            CASE WHEN p.seq_name IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END,
            CASE WHEN p.seq_name IS NULL THEN 'Sequence missing in prod'
                 ELSE 'Sequence parameters differ' END,
            999999::INT,
            5
        FROM (
            SELECT c.relname AS seq_name,
                   s.*
            FROM pg_class c
            JOIN pg_sequence s ON s.seqrelid = c.oid
            WHERE c.relnamespace = dev::regnamespace
              AND c.relkind = 'S'
        ) d
        LEFT JOIN (
            SELECT c.relname AS seq_name,
                   s.*
            FROM pg_class c
            JOIN pg_sequence s ON s.seqrelid = c.oid
            WHERE c.relnamespace = prod::regnamespace
              AND c.relkind = 'S'
        ) p ON d.seq_name = p.seq_name
        WHERE p.seq_name IS NULL 
           OR d.seqstart != p.seqstart
           OR d.seqincrement != p.seqincrement
           OR d.seqmax != p.seqmax
           OR d.seqmin != p.seqmin
           OR d.seqcache != p.seqcache
           OR d.seqcycle != p.seqcycle
        
        UNION ALL
        
        -- 3.6 Triggers
        SELECT 
            'TRIGGER'::TEXT,
            (d.table_name || '.' || d.trigger_name)::TEXT,
            CASE WHEN p.trigger_name IS NULL THEN 'MISSING' 
                 ELSE 'DIFFERENT' END,
            CASE WHEN p.trigger_name IS NULL THEN 'Trigger missing in prod'
                 ELSE 'Trigger definition differs' END,
            COALESCE(td.rn, 999999)::INT,
            3
        FROM (
            SELECT t.tgname AS trigger_name,
                   c.relname AS table_name,
                   pg_get_triggerdef(t.oid) AS trigger_def
            FROM pg_trigger t
            JOIN pg_class c ON c.oid = t.tgrelid
            WHERE c.relnamespace = dev::regnamespace
              AND NOT t.tgisinternal
        ) d
        LEFT JOIN (
            SELECT t.tgname AS trigger_name,
                   c.relname AS table_name,
                   pg_get_triggerdef(t.oid) AS trigger_def
            FROM pg_trigger t
            JOIN pg_class c ON c.oid = t.tgrelid
            WHERE c.relnamespace = prod::regnamespace
              AND NOT t.tgisinternal
        ) p ON d.table_name = p.table_name 
            AND d.trigger_name = p.trigger_name
        LEFT JOIN table_deps td ON d.table_name = td.relname
        WHERE p.trigger_name IS NULL OR d.trigger_def != p.trigger_def
        UNION ALL
        -- 3.7 Extensions (as PostgreSQL equivalent of packages)
        SELECT 
            'EXTENSION'::TEXT,
            d.extname::TEXT,
            CASE WHEN p.extname IS NULL THEN 'MISSING' 
                ELSE 'DIFFERENT' END,
            CASE WHEN p.extname IS NULL THEN 'Extension missing in prod'
                ELSE 'Extension version differs' END,
            999999::INT,
            7
        FROM (
            SELECT extname, extversion 
            FROM pg_extension 
            WHERE extnamespace = dev::regnamespace
        ) d
        LEFT JOIN (
            SELECT extname, extversion 
            FROM pg_extension 
            WHERE extnamespace = prod::regnamespace
        ) p ON d.extname = p.extname
        WHERE p.extname IS NULL OR d.extversion != p.extversion
    )
    SELECT 
        r.result_obj_type,
        r.result_obj_name,
        r.result_issue_type,
        r.result_issue_details,
        r.result_creation_order
    FROM all_results r
    ORDER BY r.type_order, r.result_creation_order, r.result_obj_name;
END;
$$;

DROP SCHEMA IF EXISTS dev CASCADE;
DROP SCHEMA IF EXISTS prod CASCADE;

CREATE SCHEMA dev;
CREATE SCHEMA prod;

CREATE TABLE dev.users (id SERIAL PRIMARY KEY, name TEXT, email TEXT);
CREATE TABLE dev.orders (id SERIAL PRIMARY KEY, user_id INT REFERENCES dev.users(id), amount DECIMAL);

CREATE TABLE prod.users (id SERIAL PRIMARY KEY, name TEXT);

CREATE INDEX idx_dev_users_email ON dev.users(email);

CREATE OR REPLACE FUNCTION dev.calculate_total(amount DECIMAL)
RETURNS DECIMAL AS $$
BEGIN
    RETURN amount * 1.1;
END;
$$ LANGUAGE plpgsql;

CREATE VIEW dev.user_orders AS
SELECT u.name, COUNT(o.id) as order_count
FROM dev.users u
LEFT JOIN dev.orders o ON u.id = o.user_id
GROUP BY u.name;

CREATE OR REPLACE FUNCTION dev.update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

ALTER TABLE dev.users ADD COLUMN updated_at TIMESTAMP;

CREATE TRIGGER trg_update_users
BEFORE UPDATE ON dev.users
FOR EACH ROW
EXECUTE FUNCTION dev.update_timestamp();

SELECT * FROM compare_schemas_extended('dev', 'prod');