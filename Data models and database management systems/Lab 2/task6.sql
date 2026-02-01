CREATE OR REPLACE FUNCTION update_group_count()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        UPDATE Groups 
        SET c_val = c_val + 1 
        WHERE id = NEW.group_id;
    
    ELSIF TG_OP = 'UPDATE' AND OLD.group_id IS DISTINCT FROM NEW.group_id THEN
        UPDATE Groups 
        SET c_val = c_val - 1 
        WHERE id = OLD.group_id;
        
        UPDATE Groups 
        SET c_val = c_val + 1 
        WHERE id = NEW.group_id;
    END IF;
    
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS students_count_trigger ON Students;
CREATE TRIGGER students_count_trigger
AFTER INSERT OR UPDATE OF group_id ON Students
FOR EACH ROW
EXECUTE FUNCTION update_group_count();

CREATE OR REPLACE FUNCTION recalc_group_count_on_delete()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE Groups 
    SET c_val = (SELECT COUNT(*) FROM Students WHERE group_id = OLD.group_id)
    WHERE id = OLD.group_id;
    
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS students_delete_count_trigger ON Students;
CREATE TRIGGER students_delete_count_trigger
AFTER DELETE ON Students
FOR EACH ROW
EXECUTE FUNCTION recalc_group_count_on_delete();

-- Turn off cascade delete trigger
ALTER TABLE Groups DISABLE TRIGGER cascade_delete_students_trigger;

DELETE FROM Students;
DELETE FROM Groups;

INSERT INTO Groups (name) VALUES ('Группа 101'), ('Группа 102');

INSERT INTO Students (name, group_id) VALUES 
    ('Иван Иванов', 1),
    ('Петр Петров', 1),
    ('Мария Сидорова', 2);

SELECT 'AFTER INSERT:' as info, * FROM Groups;

UPDATE Students SET group_id = 2 WHERE name = 'Иван Иванов';
SELECT 'AFTER UPDATE (смена группы):' as info, * FROM Groups;

DELETE FROM Students WHERE name = 'Петр Петров';
SELECT 'AFTER DELETE:' as info, * FROM Groups;