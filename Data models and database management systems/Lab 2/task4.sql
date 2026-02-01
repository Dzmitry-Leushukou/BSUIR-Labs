CREATE TABLE IF NOT EXISTS students_audit (
    id SERIAL PRIMARY KEY,           
    operation CHAR(1) NOT NULL,            
    changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, 
    student_id INTEGER,                    
    old_name VARCHAR(255),                 
    new_name VARCHAR(255),                 
    old_group_id INTEGER,                  
    new_group_id INTEGER                   
);



CREATE OR REPLACE FUNCTION log_students_changes()
RETURNS TRIGGER AS $$
BEGIN
    IF (TG_OP = 'INSERT') THEN
        INSERT INTO students_audit 
            (operation, student_id, new_name, new_group_id)
        VALUES 
            ('I', NEW.id, NEW.name, NEW.group_id);
    
    ELSIF (TG_OP = 'DELETE') THEN
        INSERT INTO students_audit 
            (operation, student_id, old_name, old_group_id)
        VALUES 
            ('D', OLD.id, OLD.name, OLD.group_id);
    
    ELSIF (TG_OP = 'UPDATE') THEN
        INSERT INTO students_audit 
            (operation, student_id, 
             old_name, new_name, 
             old_group_id, new_group_id)
        VALUES 
            ('U', NEW.id, 
             OLD.name, NEW.name, 
             OLD.group_id, NEW.group_id);
    END IF;
    
    RETURN NULL; 
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS students_audit_trigger ON Students;

CREATE TRIGGER students_audit_trigger
AFTER INSERT OR UPDATE OR DELETE ON Students
FOR EACH ROW
EXECUTE FUNCTION log_students_changes();





DELETE FROM Groups;
DELETE FROM Students;

INSERT INTO Groups (name) VALUES 
    ('Группа 101'), 
    ('Группа 102');  

INSERT INTO Students (name, group_id) VALUES 
    ('Иван Иванов', 1),
    ('Петр Петров', 1);

UPDATE Students 
SET name = 'Иван Петров', group_id = 2 
WHERE id = 1;

DELETE FROM Students WHERE id = 2;

SELECT 
    id,
    CASE operation 
        WHEN 'I' THEN 'INSERT'
        WHEN 'U' THEN 'UPDATE' 
        WHEN 'D' THEN 'DELETE' 
    END as operation,
    changed_at,
    student_id,
    old_name,
    new_name,
    old_group_id,
    new_group_id
FROM students_audit 
ORDER BY changed_at DESC;