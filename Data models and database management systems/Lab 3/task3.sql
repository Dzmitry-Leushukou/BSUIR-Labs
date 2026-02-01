DROP FUNCTION IF EXISTS generate_migration_script(TEXT, TEXT);
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
    -- Проверка существования схем
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev) THEN
        RAISE EXCEPTION 'Schema % does not exist', dev;
    END IF;
    
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod) THEN
        RAISE EXCEPTION 'Schema % does not exist', prod;
    END IF;

    -- 1. Проверка циклических зависимостей в таблицах
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

    -- 2. Получение зависимостей таблиц для топологической сортировки
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

    -- 3. Сравнение различных типов объектов
    RETURN QUERY
    WITH all_results AS (
        -- 3.1 Таблицы
        SELECT 
            'TABLE'::TEXT as result_obj_type,
            d.relname::TEXT as result_obj_name,
            CASE WHEN p.relname IS NULL THEN 'MISSING' 
                 WHEN d.sig != p.sig THEN 'DIFFERENT'
                 ELSE 'OK' END as result_issue_type,
            CASE WHEN p.relname IS NULL THEN 'Table missing in prod'
                 WHEN d.sig != p.sig THEN 'Column structure differs'
                 ELSE 'No differences' END as result_issue_details,
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
        
        -- 3.2 Индексы
        SELECT 
            'INDEX'::TEXT,
            (d.table_name || '.' || d.index_name)::TEXT,
            CASE WHEN p.index_name IS NULL THEN 'MISSING' 
                 WHEN d.index_def != p.index_def THEN 'DIFFERENT'
                 ELSE 'OK' END,
            CASE WHEN p.index_name IS NULL THEN 'Index missing in prod'
                 WHEN d.index_def != p.index_def THEN 'Index definition differs'
                 ELSE 'No differences' END,
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
        
        -- 3.3 Функции и процедуры
        SELECT 
            d.func_type,
            (d.func_name || '(' || d.args || ')')::TEXT,
            CASE WHEN p.func_name IS NULL THEN 'MISSING' 
                 WHEN d.func_def != p.func_def THEN 'DIFFERENT'
                 ELSE 'OK' END,
            CASE WHEN p.func_name IS NULL THEN 'Function/Procedure missing in prod'
                 WHEN d.func_def != p.func_def THEN 'Function/Procedure definition differs'
                 ELSE 'No differences' END,
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
        
        -- 3.4 Представления
        SELECT 
            'VIEW'::TEXT,
            d.view_name::TEXT,
            CASE WHEN p.view_name IS NULL THEN 'MISSING' 
                 WHEN d.view_def != p.view_def THEN 'DIFFERENT'
                 ELSE 'OK' END,
            CASE WHEN p.view_name IS NULL THEN 'View missing in prod'
                 WHEN d.view_def != p.view_def THEN 'View definition differs'
                 ELSE 'No differences' END,
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
        
        -- 3.5 Последовательности
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
        
        -- 3.6 Триггеры
        SELECT 
            'TRIGGER'::TEXT,
            (d.table_name || '.' || d.trigger_name)::TEXT,
            CASE WHEN p.trigger_name IS NULL THEN 'MISSING' 
                 WHEN d.trigger_def != p.trigger_def THEN 'DIFFERENT'
                 ELSE 'OK' END,
            CASE WHEN p.trigger_name IS NULL THEN 'Trigger missing in prod'
                 WHEN d.trigger_def != p.trigger_def THEN 'Trigger definition differs'
                 ELSE 'No differences' END,
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
    )
    SELECT 
        r.result_obj_type,
        r.result_obj_name,
        r.result_issue_type,
        r.result_issue_details,
        r.result_creation_order
    FROM all_results r
    WHERE r.result_issue_type != 'OK'
    ORDER BY r.type_order, r.result_creation_order, r.result_obj_name;
END;
$$;

-- Новая функция для генерации DDL-скрипта миграции
CREATE OR REPLACE FUNCTION generate_migration_script(
    dev_schema TEXT, 
    prod_schema TEXT,
    drop_orphans BOOLEAN DEFAULT FALSE
) 
RETURNS TABLE(
    sql_command TEXT,
    execution_order INT,
    description TEXT
) 
LANGUAGE plpgsql AS $$
DECLARE
    cycle_found BOOL;
    cycle_info TEXT;
BEGIN
    -- Проверка существования схем
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = dev_schema) THEN
        RAISE EXCEPTION 'Schema % does not exist', dev_schema;
    END IF;
    
    IF NOT EXISTS (SELECT 1 FROM pg_namespace WHERE nspname = prod_schema) THEN
        RAISE EXCEPTION 'Schema % does not exist', prod_schema;
    END IF;

    -- Проверка циклических зависимостей
    WITH RECURSIVE fk_cycle AS (
        SELECT conrelid::regclass AS child, confrelid::regclass AS parent,
               ARRAY[conrelid::regclass] AS path
        FROM pg_constraint 
        WHERE contype = 'f' 
          AND connamespace IN (dev_schema::regnamespace, prod_schema::regnamespace)
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
        sql_command := '-- ERROR: Circular dependencies detected: ' || cycle_info;
        execution_order := 0;
        description := 'Cannot generate migration due to circular FK dependencies';
        RETURN NEXT;
        RETURN;
    END IF;

    -- Временные таблицы для хранения объектов
    CREATE TEMP TABLE migration_commands (
        sql_command TEXT,
        execution_order INT,
        description TEXT
    ) ON COMMIT DROP;

    -- 1. Удаление объектов из prod, которых нет в dev (если разрешено)
    IF drop_orphans THEN
        -- Удаление триггеров
        INSERT INTO migration_commands
        SELECT 
            'DROP TRIGGER IF EXISTS ' || t.tgname || ' ON ' || prod_schema || '.' || c.relname || ';',
            100,
            'Drop orphan trigger'
        FROM pg_trigger t
        JOIN pg_class c ON c.oid = t.tgrelid
        WHERE c.relnamespace = prod_schema::regnamespace
          AND NOT t.tgisinternal
          AND NOT EXISTS (
              SELECT 1 FROM pg_trigger t2
              JOIN pg_class c2 ON c2.oid = t2.tgrelid
              WHERE c2.relnamespace = dev_schema::regnamespace
                AND t2.tgname = t.tgname
                AND c2.relname = c.relname
          );

        -- Удаление индексов (кроме PK и уникальных)
        INSERT INTO migration_commands
        SELECT 
            'DROP INDEX IF EXISTS ' || prod_schema || '.' || i.relname || ';',
            200,
            'Drop orphan index'
        FROM pg_index idx
        JOIN pg_class i ON i.oid = idx.indexrelid
        JOIN pg_class c ON c.oid = idx.indrelid
        WHERE i.relnamespace = prod_schema::regnamespace
          AND i.relkind = 'i'
          AND NOT idx.indisprimary
          AND NOT idx.indisunique
          AND NOT EXISTS (
              SELECT 1 FROM pg_index idx2
              JOIN pg_class i2 ON i2.oid = idx2.indexrelid
              JOIN pg_class c2 ON c2.oid = idx2.indrelid
              WHERE i2.relnamespace = dev_schema::regnamespace
                AND i2.relname = i.relname
                AND c2.relname = c.relname
          );

        -- Удаление представлений
        INSERT INTO migration_commands
        SELECT 
            'DROP VIEW IF EXISTS ' || prod_schema || '.' || c.relname || ' CASCADE;',
            300,
            'Drop orphan view'
        FROM pg_class c
        WHERE c.relnamespace = prod_schema::regnamespace
          AND c.relkind = 'v'
          AND NOT EXISTS (
              SELECT 1 FROM pg_class c2
              WHERE c2.relnamespace = dev_schema::regnamespace
                AND c2.relkind = 'v'
                AND c2.relname = c.relname
          );

        -- Удаление таблиц (в обратном порядке зависимостей)
        WITH RECURSIVE dep_order AS (
            SELECT c.oid, c.relname, 1 as lvl
            FROM pg_class c
            WHERE c.relnamespace = prod_schema::regnamespace
              AND c.relkind = 'r'
              AND NOT EXISTS (
                  SELECT 1 FROM pg_constraint fk 
                  WHERE fk.confrelid = c.oid
              )
            UNION
            SELECT c.oid, c.relname, d.lvl + 1
            FROM dep_order d
            JOIN pg_constraint fk ON fk.conrelid = d.oid
            JOIN pg_class c ON c.oid = fk.confrelid
        )
        INSERT INTO migration_commands
        SELECT 
            'DROP TABLE IF EXISTS ' || prod_schema || '.' || d.relname || ' CASCADE;',
            400 + (SELECT MAX(lvl) FROM dep_order) - d.lvl,
            'Drop orphan table'
        FROM dep_order d
        WHERE NOT EXISTS (
            SELECT 1 FROM pg_class c2
            WHERE c2.relnamespace = dev_schema::regnamespace
              AND c2.relkind = 'r'
              AND c2.relname = d.relname
        );
    END IF;

    -- 2. Создание/изменение таблиц
    -- Сначала создаем недостающие таблицы
    INSERT INTO migration_commands
    SELECT 
        'CREATE TABLE ' || prod_schema || '.' || c.relname || E' (\n' ||
        STRING_AGG(
            '    ' || a.attname || ' ' || 
            format_type(a.atttypid, a.atttypmod) || ' ' ||
            CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE '' END,
            E',\n'
            ORDER BY a.attnum
        ) || E'\n);',
        1000 + COALESCE(td.rn, 0),
        'Create missing table'
    FROM pg_class c
    JOIN pg_attribute a ON a.attrelid = c.oid
    LEFT JOIN (
        WITH RECURSIVE topo AS (
            SELECT c.oid, c.relname, 1 AS lvl
            FROM pg_class c
            WHERE c.relnamespace = prod_schema::regnamespace 
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
          GROUP BY relname
    ) td ON c.relname = td.relname
    WHERE c.relnamespace = dev_schema::regnamespace
      AND c.relkind = 'r'
      AND a.attnum > 0
      AND NOT a.attisdropped
      AND NOT EXISTS (
          SELECT 1 FROM pg_class c2
          WHERE c2.relnamespace = prod_schema::regnamespace
            AND c2.relkind = 'r'
            AND c2.relname = c.relname
      )
    GROUP BY c.relname, td.rn;

    -- Затем добавляем отсутствующие колонки
    INSERT INTO migration_commands
    SELECT 
        'ALTER TABLE ' || prod_schema || '.' || c.relname || 
        ' ADD COLUMN ' || a.attname || ' ' || 
        format_type(a.atttypid, a.atttypmod) || ' ' ||
        CASE WHEN a.attnotnull THEN 'NOT NULL' ELSE '' END || ';',
        1100,
        'Add missing column to table'
    FROM pg_class c
    JOIN pg_attribute a ON a.attrelid = c.oid
    WHERE c.relnamespace = dev_schema::regnamespace
      AND c.relkind = 'r'
      AND a.attnum > 0
      AND NOT a.attisdropped
      AND EXISTS (
          SELECT 1 FROM pg_class c2
          WHERE c2.relnamespace = prod_schema::regnamespace
            AND c2.relkind = 'r'
            AND c2.relname = c.relname
      )
      AND NOT EXISTS (
          SELECT 1 FROM pg_attribute a2
          JOIN pg_class c2 ON c2.oid = a2.attrelid
          WHERE c2.relnamespace = prod_schema::regnamespace
            AND c2.relname = c.relname
            AND a2.attname = a.attname
            AND NOT a2.attisdropped
      );

    -- 3. Создание индексов
    INSERT INTO migration_commands
    SELECT 
        pg_get_indexdef(i.oid) || ';',
        2000,
        'Create missing index'
    FROM pg_index idx
    JOIN pg_class i ON i.oid = idx.indexrelid
    JOIN pg_class c ON c.oid = idx.indrelid
    WHERE i.relnamespace = dev_schema::regnamespace
      AND i.relkind = 'i'
      AND NOT EXISTS (
          SELECT 1 FROM pg_index idx2
          JOIN pg_class i2 ON i2.oid = idx2.indexrelid
          JOIN pg_class c2 ON c2.oid = idx2.indrelid
          WHERE i2.relnamespace = prod_schema::regnamespace
            AND i2.relname = i.relname
            AND c2.relname = c.relname
      );

    -- 4. Создание функций/процедур
    INSERT INTO migration_commands
    SELECT 
        pg_get_functiondef(p.oid) || ';',
        3000,
        'Create/replace function'
    FROM pg_proc p
    WHERE p.pronamespace = dev_schema::regnamespace
      AND NOT EXISTS (
          SELECT 1 FROM pg_proc p2
          WHERE p2.pronamespace = prod_schema::regnamespace
            AND p2.proname = p.proname
            AND pg_get_function_arguments(p2.oid) = pg_get_function_arguments(p.oid)
            AND pg_get_functiondef(p2.oid) = pg_get_functiondef(p.oid)
      );

    -- 5. Создание представлений
    INSERT INTO migration_commands
    SELECT 
        'CREATE OR REPLACE VIEW ' || prod_schema || '.' || c.relname || 
        ' AS ' || pg_get_viewdef(c.oid) || ';',
        4000,
        'Create/replace view'
    FROM pg_class c
    WHERE c.relnamespace = dev_schema::regnamespace
      AND c.relkind = 'v'
      AND NOT EXISTS (
          SELECT 1 FROM pg_class c2
          WHERE c2.relnamespace = prod_schema::regnamespace
            AND c2.relkind = 'v'
            AND c2.relname = c.relname
            AND pg_get_viewdef(c2.oid) = pg_get_viewdef(c.oid)
      );

    -- 6. Создание триггеров
    INSERT INTO migration_commands
    SELECT 
        pg_get_triggerdef(t.oid) || ';',
        5000,
        'Create/replace trigger'
    FROM pg_trigger t
    JOIN pg_class c ON c.oid = t.tgrelid
    WHERE c.relnamespace = dev_schema::regnamespace
      AND NOT t.tgisinternal
      AND NOT EXISTS (
          SELECT 1 FROM pg_trigger t2
          JOIN pg_class c2 ON c2.oid = t2.tgrelid
          WHERE c2.relnamespace = prod_schema::regnamespace
            AND t2.tgname = t.tgname
            AND c2.relname = c.relname
            AND pg_get_triggerdef(t2.oid) = pg_get_triggerdef(t.oid)
      );

    -- Возвращаем команды в правильном порядке
    RETURN QUERY
    SELECT 
        mc.sql_command,
        mc.execution_order,
        mc.description
    FROM migration_commands mc
    ORDER BY mc.execution_order, mc.sql_command;

END;
$$;

-- Тестовые схемы и объекты
DROP SCHEMA IF EXISTS dev CASCADE;
DROP SCHEMA IF EXISTS prod CASCADE;

CREATE SCHEMA dev;
CREATE SCHEMA prod;

-- Создаем таблицы в dev
CREATE TABLE dev.users (
    id SERIAL PRIMARY KEY, 
    name TEXT NOT NULL, 
    email TEXT UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE dev.orders (
    id SERIAL PRIMARY KEY, 
    user_id INT REFERENCES dev.users(id), 
    amount DECIMAL(10,2),
    status TEXT DEFAULT 'pending'
);

-- Создаем таблицы в prod (старая версия)
CREATE TABLE prod.users (
    id SERIAL PRIMARY KEY, 
    name TEXT NOT NULL
);

-- Индексы в dev
CREATE INDEX idx_dev_users_email ON dev.users(email);
CREATE INDEX idx_dev_orders_user ON dev.orders(user_id);

-- Функции в dev
CREATE OR REPLACE FUNCTION dev.calculate_total(amount DECIMAL)
RETURNS DECIMAL AS $$
BEGIN
    RETURN amount * 1.1;
END;
$$ LANGUAGE plpgsql;

-- Представления в dev
CREATE VIEW dev.user_orders AS
SELECT u.name, COUNT(o.id) as order_count, SUM(o.amount) as total_amount
FROM dev.users u
LEFT JOIN dev.orders o ON u.id = o.user_id
GROUP BY u.name;

-- Триггеры в dev
CREATE OR REPLACE FUNCTION dev.update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.created_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_update_users_timestamp
BEFORE INSERT ON dev.users
FOR EACH ROW
EXECUTE FUNCTION dev.update_timestamp();

-- Тестируем
SELECT * FROM compare_schemas_extended('dev', 'prod');

-- Генерируем миграционный скрипт (без удаления лишних объектов)
SELECT * FROM generate_migration_script('dev', 'prod', false);

-- Генерируем миграционный скрипт (с удалением лишних объектов)
SELECT * FROM generate_migration_script('dev', 'prod', true);