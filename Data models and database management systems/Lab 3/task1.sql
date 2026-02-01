DROP FUNCTION compare_schemas(dev TEXT, prod TEXT);
CREATE OR REPLACE FUNCTION compare_schemas(dev TEXT, prod TEXT) 
RETURNS TABLE(tbl TEXT, issue TEXT, details TEXT, ord INT) 
LANGUAGE plpgsql AS $$
BEGIN
    -- 1. Check cycles in BOTH schemas
    WITH RECURSIVE fk_chain AS (
        -- Check dev schema
        SELECT conrelid::regclass AS child, confrelid::regclass AS parent,
               connamespace::regnamespace AS schema_name,
               ARRAY[conrelid::regclass] AS path,
               FALSE AS cycle
        FROM pg_constraint 
        WHERE contype = 'f' 
          AND connamespace IN (dev::regnamespace, prod::regnamespace)
        
        UNION ALL
        
        SELECT c.conrelid::regclass, c.confrelid::regclass, 
               c.connamespace::regnamespace,
               fc.path || c.conrelid::regclass,
               c.conrelid::regclass = ANY(fc.path)
        FROM pg_constraint c
        JOIN fk_chain fc ON c.confrelid = fc.child 
          AND c.connamespace = fc.schema_name
        WHERE NOT fc.cycle
    ),
    cycles AS (
        SELECT schema_name::TEXT AS schema_name,
               STRING_AGG(DISTINCT child::TEXT, ', ') AS cycle_tables
        FROM fk_chain 
        WHERE cycle
        GROUP BY schema_name
    )
    SELECT INTO tbl, issue, details, ord
        '', 'CYCLIC_DEPENDENCY', 
        CASE 
            WHEN EXISTS(SELECT 1 FROM cycles WHERE schema_name = dev) 
                 AND EXISTS(SELECT 1 FROM cycles WHERE schema_name = prod)
            THEN 'Both schemas have circular FK dependencies'
            WHEN EXISTS(SELECT 1 FROM cycles WHERE schema_name = dev)
            THEN 'Dev schema has circular FK dependencies: ' || 
                 (SELECT cycle_tables FROM cycles WHERE schema_name = dev)
            ELSE 'Prod schema has circular FK dependencies: ' ||
                 (SELECT cycle_tables FROM cycles WHERE schema_name = prod)
        END, 0
    FROM cycles
    LIMIT 1;
    
    IF FOUND THEN RETURN NEXT; RETURN; END IF;

    -- 2. Get ALL foreign key dependencies (from both schemas)
    CREATE TEMP TABLE all_fk ON COMMIT DROP AS
    SELECT 
        c.oid AS child_id,
        c.relname AS child_name,
        p.oid AS parent_id,
        p.relname AS parent_name,
        ns.nspname AS schema_name
    FROM pg_constraint con
    JOIN pg_class c ON c.oid = con.conrelid
    JOIN pg_class p ON p.oid = con.confrelid
    JOIN pg_namespace ns ON ns.oid = c.relnamespace
    WHERE con.contype = 'f'
      AND ns.nspname IN (dev, prod)
      AND con.confrelid IN (
          SELECT oid FROM pg_class 
          WHERE relnamespace IN (dev::regnamespace, prod::regnamespace)
      );

    -- 3. Topological sort for ALL tables in prod (considering dependencies from both schemas)
    CREATE TEMP TABLE all_dependencies ON COMMIT DROP AS
    WITH RECURSIVE topo_sort AS (
        -- Start with tables that have no FK dependencies
        SELECT 
            c.oid,
            c.relname AS table_name,
            1 AS level,
            ARRAY[c.relname] AS path
        FROM pg_class c
        JOIN pg_namespace ns ON ns.oid = c.relnamespace
        WHERE ns.nspname = prod
          AND c.relkind = 'r'
          AND NOT EXISTS (
              SELECT 1 FROM all_fk fk
              WHERE fk.child_name = c.relname
                AND fk.schema_name = prod
          )
        
        UNION ALL
        
        -- Add dependent tables (considering dependencies from both schemas)
        SELECT 
            c.oid,
            c.relname,
            ts.level + 1,
            ts.path || c.relname
        FROM topo_sort ts
        JOIN all_fk fk ON fk.parent_name = ts.table_name
        JOIN pg_class c ON c.relname = fk.child_name
        JOIN pg_namespace ns ON ns.oid = c.relnamespace
        WHERE ns.nspname = prod
          AND c.relname <> ALL(ts.path)
    )
    SELECT 
        table_name,
        ROW_NUMBER() OVER (ORDER BY MIN(level), table_name) AS sort_order
    FROM topo_sort
    GROUP BY table_name;

    -- 4. Compare schemas
    RETURN QUERY
    WITH dev_tbls AS (
        SELECT c.relname, 
               STRING_AGG(a.attname || ':' || 
                         format_type(a.atttypid, a.atttypmod) || ':' ||
                         CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE 'NULL' END, 
                         '|' ORDER BY a.attnum) AS sig
        FROM pg_class c
        JOIN pg_attribute a ON a.attrelid = c.oid
        JOIN pg_namespace ns ON ns.oid = c.relnamespace
        WHERE ns.nspname = dev
          AND c.relkind = 'r'
          AND a.attnum > 0 
          AND NOT a.attisdropped
        GROUP BY c.relname
    ),
    prod_tbls AS (
        SELECT c.relname, 
               STRING_AGG(a.attname || ':' || 
                         format_type(a.atttypid, a.atttypmod) || ':' ||
                         CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE 'NULL' END, 
                         '|' ORDER BY a.attnum) AS sig
        FROM pg_class c
        JOIN pg_attribute a ON a.attrelid = c.oid
        JOIN pg_namespace ns ON ns.oid = c.relnamespace
        WHERE ns.nspname = prod
          AND c.relkind = 'r'
          AND a.attnum > 0 
          AND NOT a.attisdropped
        GROUP BY c.relname
    ),
    -- For missing tables, estimate their possible position based on dev dependencies
    missing_with_order AS (
        SELECT 
            d.relname,
            -- Estimate order: if table has no dependencies in dev, put it early
            -- If it depends on tables that exist in prod, put it after them
            CASE 
                WHEN EXISTS (
                    SELECT 1 FROM all_fk fk 
                    WHERE fk.child_name = d.relname 
                      AND fk.schema_name = dev
                      AND fk.parent_name IN (SELECT table_name FROM all_dependencies)
                ) THEN (
                    SELECT MAX(ad.sort_order) + 1
                    FROM all_fk fk
                    JOIN all_dependencies ad ON ad.table_name = fk.parent_name
                    WHERE fk.child_name = d.relname 
                      AND fk.schema_name = dev
                )
                ELSE (
                    SELECT COALESCE(MAX(sort_order), 0) + 1 
                    FROM all_dependencies
                )
            END AS estimated_order
        FROM dev_tbls d
        LEFT JOIN prod_tbls p ON d.relname = p.relname
        WHERE p.relname IS NULL
    )
    SELECT 
        d.relname::TEXT,
        CASE 
            WHEN p.relname IS NULL THEN 'MISSING_TABLE' 
            ELSE 'DIFFERENT_STRUCTURE' 
        END,
        CASE 
            WHEN p.relname IS NULL THEN 'Table exists in dev but not in prod'
            ELSE 'Column structure differs between schemas' 
        END,
        COALESCE(
            ad.sort_order, 
            mwo.estimated_order,
            999999
        )::INT
    FROM dev_tbls d
    LEFT JOIN prod_tbls p ON d.relname = p.relname
    LEFT JOIN all_dependencies ad ON d.relname = ad.table_name
    LEFT JOIN missing_with_order mwo ON d.relname = mwo.relname
    WHERE p.relname IS NULL OR d.sig != p.sig
    ORDER BY 
        COALESCE(ad.sort_order, mwo.estimated_order, 999999),
        d.relname;
END;
$$;



DROP SCHEMA IF EXISTS dev CASCADE;
DROP SCHEMA IF EXISTS prod CASCADE;

CREATE SCHEMA dev;
CREATE SCHEMA prod;

-- В dev обе таблицы
CREATE TABLE dev.users (id SERIAL PRIMARY KEY, name TEXT, email TEXT);
CREATE TABLE dev.orders (id SERIAL PRIMARY KEY, user_id INT REFERENCES dev.users(id), amount DECIMAL);

-- В prod только users, причем без email
CREATE TABLE prod.users (id SERIAL PRIMARY KEY, name TEXT);

-- Run comparison
SELECT * FROM compare_schemas('dev', 'prod');