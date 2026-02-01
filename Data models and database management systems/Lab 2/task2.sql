DROP TRIGGER IF EXISTS unique_sid_trigger ON Students;
DROP TRIGGER IF EXISTS autoincrement_student_id_trigger ON Students;

DROP TRIGGER IF EXISTS unique_gid_trigger ON Groups;
DROP TRIGGER IF EXISTS autoincrement_group_id_trigger ON Groups;
DROP TRIGGER IF EXISTS unique_group_name_trigger ON Groups;


CREATE OR REPLACE FUNCTION check_unique_sid()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (SELECT 1 FROM Students WHERE id = NEW.id) THEN
        RAISE EXCEPTION 'Student with id % already exists (duplicate)', NEW.id;
    END IF;
    
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION check_unique_gid()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (SELECT 1 FROM Groups WHERE id = NEW.id) THEN
        RAISE EXCEPTION 'Group with id % already exists (duplicate)', NEW.id;
    END IF;
    
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE TRIGGER unique_sid_trigger
BEFORE INSERT ON Students
FOR EACH ROW
EXECUTE FUNCTION check_unique_sid();

CREATE TRIGGER unique_gid_trigger
BEFORE INSERT ON Groups
FOR EACH ROW
EXECUTE FUNCTION check_unique_gid();



CREATE OR REPLACE FUNCTION generate_student_id()
RETURNS TRIGGER AS $$
DECLARE
    max_id INTEGER;
BEGIN
   IF NEW.id IS NULL THEN
        SELECT COALESCE(MAX(id), 0) INTO max_id FROM Students;
        NEW.id := max_id + 1;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER autoincrement_student_id_trigger
BEFORE INSERT ON Students
FOR EACH ROW
EXECUTE FUNCTION generate_student_id();


CREATE OR REPLACE FUNCTION generate_group_id()
RETURNS TRIGGER AS $$
DECLARE
    max_id INTEGER;
BEGIN
    IF NEW.id IS NULL THEN
        SELECT COALESCE(MAX(id), 0) INTO max_id FROM Groups;
        NEW.id := max_id + 1;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER autoincrement_group_id_trigger
BEFORE INSERT ON Groups
FOR EACH ROW
EXECUTE FUNCTION generate_group_id();



CREATE OR REPLACE FUNCTION check_unique_group_name()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (SELECT 1 FROM Groups WHERE name = NEW.name AND id != NEW.id) THEN
        RAISE EXCEPTION 'Группа с названием "%" уже существует', NEW.name;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER unique_group_name_trigger
BEFORE INSERT OR UPDATE ON Groups
FOR EACH ROW
EXECUTE FUNCTION check_unique_group_name();












INSERT INTO Groups (name) VALUES ('Группа 101'); -- ID=1
INSERT INTO Groups (name) VALUES ('Группа 102'); -- ID=2
INSERT INTO Groups (id, name) VALUES (1, 'Группа 103'); -- Error

INSERT INTO Students (name, group_id) VALUES ('Иван', 1); -- ID=1
INSERT INTO Students (name, group_id) VALUES ('Петр', 1); -- ID=2
INSERT INTO Students (id, name, group_id) VALUES (1, 'Дубликат', 1); -- Error