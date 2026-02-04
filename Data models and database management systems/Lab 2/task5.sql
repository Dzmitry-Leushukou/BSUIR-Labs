CREATE OR REPLACE PROCEDURE restore_students(
    target_time TIMESTAMP
)
LANGUAGE plpgsql
AS $$
BEGIN
    RAISE NOTICE 'Restored students to %', target_time;
    
    TRUNCATE TABLE Students;
    
    INSERT INTO Students (id, name, group_id)
    SELECT 
        a.student_id,
        a.last_name,
        a.last_group_id
    FROM (
        SELECT 
            student_id,
            CASE 
                WHEN operation = 'D' THEN NULL
                ELSE COALESCE(new_name, old_name)
            END as last_name,
            CASE 
                WHEN operation = 'D' THEN NULL
                ELSE COALESCE(new_group_id, old_group_id)
            END as last_group_id,
            changed_at,
            operation,
            ROW_NUMBER() OVER (
                PARTITION BY student_id 
                ORDER BY changed_at DESC
            ) as rnk
        FROM students_audit
        WHERE changed_at <= target_time
    ) a
    WHERE a.rnk = 1
      AND a.last_name IS NOT NULL
      AND a.last_group_id IS NOT NULL
      AND a.operation != 'D';
    
    RAISE NOTICE 'Restored % students', (SELECT COUNT(*) FROM Students);
END;
$$;

CREATE OR REPLACE PROCEDURE restore_students_by_interval(
    interval_text TEXT  -- '5 minutes', '1 hour', '2 days'
)
LANGUAGE plpgsql
AS $$
DECLARE
    target_time TIMESTAMP;
BEGIN
    target_time := CURRENT_TIMESTAMP - interval_text::INTERVAL;
    
    RAISE NOTICE 'Restore data for interval: % ago (target time: %)', 
                 interval_text, target_time;
    
    CALL restore_students(target_time);
END;
$$;

-- DELETE FROM Groups;
-- DELETE FROM Students;
-- DELETE FROM students_audit;

-- INSERT INTO Groups (name) VALUES ('Группа 101');
-- INSERT INTO Groups (name) VALUES ('Группа 102');
INSERT INTO Students (name, group_id) VALUES ('Иван Иванов', 1);
INSERT INTO Students (name, group_id) VALUES ('Пётр Иванов', 1);
INSERT INTO Students (name, group_id) VALUES ('Пётр Петров', 2);
INSERT INTO Students (name, group_id) VALUES ('Александр Фёдоров', 2);

call restore_students_by_interval('2 minutes');

SELECT * FROM students_audit;
SELECT * FROM students;

UPDATE Students SET name = '12312 Петров' WHERE id = 5;
