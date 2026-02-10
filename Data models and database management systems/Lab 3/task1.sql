-- 1. Создаем вспомогательные функции

-- Функция для получения структуры таблицы в удобном формате
CREATE OR REPLACE FUNCTION get_table_structure(schema_name TEXT, table_name TEXT)
RETURNS TABLE (
    column_name TEXT,
    data_type TEXT,
    is_nullable TEXT,
    column_default TEXT,
    character_maximum_length INTEGER,
    numeric_precision INTEGER,
    numeric_scale INTEGER
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        c.column_name::TEXT,
        CASE 
            WHEN c.data_type = 'USER-DEFINED' THEN c.udt_name::TEXT
            ELSE c.data_type::TEXT
        END,
        c.is_nullable::TEXT,
        COALESCE(c.column_default::TEXT, ''),
        c.character_maximum_length,
        c.numeric_precision,
        c.numeric_scale
    FROM information_schema.columns c
    WHERE c.table_schema = schema_name 
        AND c.table_name = table_name
    ORDER BY c.ordinal_position;
END;
$$ LANGUAGE plpgsql;

-- 2. Основная процедура сравнения схем
CREATE OR REPLACE PROCEDURE compare_schemas_tables(
    dev_schema_name TEXT,
    prod_schema_name TEXT,
    INOUT result_table REFCURSOR = 'result_cursor'
)
LANGUAGE plpgsql
AS $$
DECLARE
    table_record RECORD;
    dev_column_count INTEGER;
    prod_column_count INTEGER;
    column_mismatch_count INTEGER;
    dependency_graph TEXT[];
    cyclic_dependency BOOLEAN := FALSE;
    processed_tables TEXT[] := '{}';
    
BEGIN
    -- Временная таблица для хранения результатов
    CREATE TEMP TABLE IF NOT EXISTS comparison_results (
        table_name TEXT,
        status TEXT,
        issue_description TEXT,
        creation_order INTEGER DEFAULT 0
    );
    
    TRUNCATE TABLE comparison_results;
    
    -- Находим таблицы, которые есть в Dev, но нет в Prod
    FOR table_record IN 
        SELECT table_name 
        FROM information_schema.tables 
        WHERE table_schema = dev_schema_name 
            AND table_type = 'BASE TABLE'
        EXCEPT
        SELECT table_name 
        FROM information_schema.tables 
        WHERE table_schema = prod_schema_name 
            AND table_type = 'BASE TABLE'
    LOOP
        INSERT INTO comparison_results (table_name, status, issue_description)
        VALUES (table_record.table_name, 'MISSING', 'Table exists only in Dev schema');
    END LOOP;
    
    -- Находим таблицы, которые есть в обеих схемах, но с разной структурой
    FOR table_record IN 
        SELECT t1.table_name
        FROM information_schema.tables t1
        JOIN information_schema.tables t2 
            ON t1.table_name = t2.table_name
        WHERE t1.table_schema = dev_schema_name 
            AND t2.table_schema = prod_schema_name
            AND t1.table_type = 'BASE TABLE' 
            AND t2.table_type = 'BASE TABLE'
    LOOP
        -- Проверяем количество столбцов
        SELECT COUNT(*) INTO dev_column_count
        FROM information_schema.columns
        WHERE table_schema = dev_schema_name 
            AND table_name = table_record.table_name;
            
        SELECT COUNT(*) INTO prod_column_count
        FROM information_schema.columns
        WHERE table_schema = prod_schema_name 
            AND table_name = table_record.table_name;
        
        -- Если разное количество столбцов
        IF dev_column_count != prod_column_count THEN
            INSERT INTO comparison_results (table_name, status, issue_description)
            VALUES (table_record.table_name, 'STRUCTURE_DIFF', 
                    'Different column count: Dev=' || dev_column_count || ', Prod=' || prod_column_count);
        ELSE
            -- Проверяем различия в столбцах
            SELECT COUNT(*) INTO column_mismatch_count
            FROM (
                SELECT column_name, data_type, is_nullable, column_default
                FROM get_table_structure(dev_schema_name, table_record.table_name)
                EXCEPT
                SELECT column_name, data_type, is_nullable, column_default
                FROM get_table_structure(prod_schema_name, table_record.table_name)
            ) AS diff;
            
            IF column_mismatch_count > 0 THEN
                INSERT INTO comparison_results (table_name, status, issue_description)
                VALUES (table_record.table_name, 'STRUCTURE_DIFF', 
                        'Column mismatch count: ' || column_mismatch_count);
            END IF;
        END IF;
    END LOOP;
    
    -- Определяем порядок создания таблиц на основе foreign keys
    WITH RECURSIVE table_deps AS (
        -- Таблицы без зависимостей (корни)
        SELECT 
            tc.table_name,
            ARRAY[tc.table_name] AS path,
            false AS cycle
        FROM information_schema.tables tc
        WHERE tc.table_schema = dev_schema_name
            AND tc.table_type = 'BASE TABLE'
            AND NOT EXISTS (
                SELECT 1
                FROM information_schema.table_constraints fk
                JOIN information_schema.key_column_usage kcu 
                    ON fk.constraint_name = kcu.constraint_name
                WHERE fk.table_schema = dev_schema_name
                    AND fk.constraint_type = 'FOREIGN KEY'
                    AND fk.table_name = tc.table_name
            )
        
        UNION ALL
        
        -- Рекурсивно добавляем таблицы с зависимостями
        SELECT 
            tc.table_name,
            td.path || tc.table_name,
            tc.table_name = ANY(td.path)
        FROM information_schema.tables tc
        JOIN information_schema.table_constraints fk 
            ON fk.table_schema = dev_schema_name
            AND fk.table_name = tc.table_name
            AND fk.constraint_type = 'FOREIGN KEY'
        JOIN information_schema.key_column_usage kcu 
            ON fk.constraint_name = kcu.constraint_name
        JOIN information_schema.table_constraints pk 
            ON kcu.referenced_table_schema = dev_schema_name
            AND pk.table_schema = dev_schema_name
            AND pk.table_name = kcu.referenced_table_name
            AND pk.constraint_name = fk.constraint_name
        JOIN table_deps td 
            ON pk.table_name = td.table_name
        WHERE tc.table_schema = dev_schema_name
            AND tc.table_type = 'BASE TABLE'
    )
    UPDATE comparison_results cr
    SET creation_order = td.row_num
    FROM (
        SELECT 
            table_name,
            ROW_NUMBER() OVER (ORDER BY array_length(path, 1), table_name) as row_num
        FROM table_deps
        WHERE NOT cycle
        GROUP BY table_name, path
    ) td
    WHERE cr.table_name = td.table_name;
    
    -- Проверяем наличие циклических зависимостей
    SELECT EXISTS(
        SELECT 1 
        FROM (
            SELECT DISTINCT table_name
            FROM table_deps
            WHERE cycle
        ) cyclic_tables
        WHERE table_name IN (SELECT table_name FROM comparison_results)
    ) INTO cyclic_dependency;
    
    -- Открываем курсор с результатами
    OPEN result_table FOR
    SELECT 
        cr.table_name,
        cr.status,
        cr.issue_description,
        CASE 
            WHEN cr.creation_order = 0 THEN 'N/A - check dependencies'
            ELSE cr.creation_order::TEXT
        END as creation_order,
        CASE 
            WHEN cyclic_dependency AND cr.table_name IN (
                SELECT DISTINCT table_name FROM table_deps WHERE cycle
            ) THEN 'WARNING: Cyclic dependency detected'
            ELSE ''
        END as dependency_warning
    FROM comparison_results cr
    ORDER BY 
        CASE WHEN cr.creation_order = 0 THEN 999999 ELSE cr.creation_order END,
        cr.table_name;
    
    -- Дополнительное сообщение о циклических зависимостях
    IF cyclic_dependency THEN
        RAISE NOTICE 'ВНИМАНИЕ: Обнаружены циклические зависимости между таблицами. Требуется ручная проверка порядка создания таблиц.';
    END IF;
    
    DROP TABLE comparison_results;
END;
$$;


CREATE OR REPLACE FUNCTION compare_schemas_tables_func(
    dev_schema_name TEXT,
    prod_schema_name TEXT
)
RETURNS TABLE (
    table_name TEXT,
    status TEXT,
    issue_description TEXT,
    creation_order TEXT,
    dependency_warning TEXT
) AS $$
DECLARE
    result_cursor REFCURSOR;
BEGIN
    CALL compare_schemas_tables(dev_schema_name, prod_schema_name, result_cursor);
    RETURN QUERY FETCH ALL FROM result_cursor;
END;
$$ LANGUAGE plpgsql;












CREATE SCHEMA IF NOT EXISTS dev_schema;
CREATE SCHEMA IF NOT EXISTS prod_schema;

CREATE TABLE dev_schema.departments (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL
);

CREATE TABLE dev_schema.employees (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    department_id INTEGER REFERENCES dev_schema.departments(id),
    salary DECIMAL(10,2)
);

CREATE TABLE prod_schema.departments (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) NOT NULL 
);

BEGIN;
CALL compare_schemas_tables('dev_schema', 'prod_schema');
FETCH ALL FROM result_cursor;
COMMIT;


SELECT * FROM compare_schemas_tables_func('dev_schema', 'prod_schema');