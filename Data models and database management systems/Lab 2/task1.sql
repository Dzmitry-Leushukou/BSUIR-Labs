DROP TABLE IF EXISTS Students;
DROP TABLE IF EXISTS Groups;

CREATE TABLE IF NOT EXISTS Students
(
    id INTEGER,
    name VARCHAR(255),
    group_id INTEGER
);
CREATE TABLE IF NOT EXISTS Groups
(
    id INTEGER,
    name VARCHAR(255),
    c_val INTEGER default 0
);


