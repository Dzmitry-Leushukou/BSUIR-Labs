CREATE OR REPLACE FUNCTION check_group_exists()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.group_id IS NOT NULL AND 
       NOT EXISTS (SELECT 1 FROM Groups WHERE id = NEW.group_id) THEN
        RAISE EXCEPTION 'Group ID % does not exist', NEW.group_id;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS check_group_exists_trigger ON Students;
CREATE TRIGGER check_group_exists_trigger
BEFORE INSERT OR UPDATE ON Students
FOR EACH ROW
EXECUTE FUNCTION check_group_exists();

CREATE OR REPLACE FUNCTION cascade_delete_students()
RETURNS TRIGGER AS $$
BEGIN
    DELETE FROM Students WHERE group_id = OLD.id;

    RAISE NOTICE 'Cascade delete students: group_id = %', OLD.id;
    RETURN OLD;
END;
$$ LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS cascade_delete_students_trigger ON Groups;
CREATE TRIGGER cascade_delete_students_trigger
BEFORE DELETE ON Groups
FOR EACH ROW
EXECUTE FUNCTION cascade_delete_students();



INSERT INTO Groups (id, name) VALUES (1, 'Группа 103'); -- "Group with id 1 already exists (duplicate)"

INSERT INTO Students (name, group_id) VALUES ('Иван', 1); 

INSERT INTO Students (id, name, group_id) VALUES (1, 'Петр', 1); --: "Student with id 1 already exists (duplicate)"