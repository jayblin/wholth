CREATE VIRTUAL TABLE IF NOT EXISTS body_part_localisation_fts5
USING fts5(title, description);

CREATE TABLE IF NOT EXISTS body_part (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	bpl_fts5_rowid INTEGER NOT NULL,
    unique_title TEXT NOT NULL
) STRICT;

CREATE TABLE IF NOT EXISTS body_part_nset (
	body_part_id INTEGER,
    lft INTEGER NOT NULL,
    rgt INTEGER NOT NULL,
    UNIQUE(body_part_id),
	FOREIGN KEY (body_part_id) REFERENCES body_part(id)
) STRICT;

INSERT INTO body_part_localisation_fts5 (rowid, title) VALUES
(1,'тело'),
(2,'шея'),
(3,'руки'),
(4,'спина верх'),
(5,'спина низ'),
(6,'ноги')
;

INSERT INTO body_part (id, bpl_fts5_rowid, unique_title) VALUES
(1,1,'тело'),
(2,2,'шея'),
(3,3,'руки'),
(4,4,'спина верх'),
(5,5,'спина низ'),
(6,6,'ноги')
;

INSERT INTO body_part_nset (body_part_id, lft, rgt) VALUES
(1,1,12),
(2,2,3),
(3,4,5),
(4,6,7),
(5,8,9),
(6,10,11)
;

CREATE TRIGGER IF NOT EXISTS body_part_before_insert_trigger
BEFORE INSERT ON body_part
BEGIN
    WITH
    title_chk(a) AS (VALUES (NEW.unique_title))
    SELECT
        CASE
            WHEN a = ''
            THEN RAISE(FAIL, 'Недопустимое название части тела!')
        END
    FROM title_chk;
    WITH
    unq_title_chck(cnt) AS (
        SELECT
            COUNT(id)
        FROM body_part
        WHERE unique_title = NEW.unique_title
    )
    SELECT
        CASE
            WHEN cnt > 0
            THEN RAISE(FAIL, 'Уже есть часть тела с таким названием!')
        END
    FROM unq_title_chck;
END;

CREATE TRIGGER IF NOT EXISTS body_part_before_update_trigger
BEFORE UPDATE ON body_part
BEGIN
    WITH
    title_chk(a) AS (VALUES (NEW.unique_title))
    SELECT
        CASE
            WHEN a = ''
            THEN RAISE(FAIL, 'Недопустимое название части тела!')
        END
    FROM title_chk;
    WITH
    unq_title_chck(cnt) AS (
        SELECT
            COUNT(id)
        FROM body_part
        WHERE unique_title = NEW.unique_title AND id <> NEW.id
    )
    SELECT
        CASE
            WHEN cnt > 0
            THEN RAISE(FAIL, 'Уже есть часть тела с таким названием!')
        END
    FROM unq_title_chck;
END
;

CREATE TRIGGER IF NOT EXISTS body_part_nset_before_insert_trigger
BEFORE INSERT ON body_part_nset
BEGIN
    WITH
    root_chk(cnt) AS (
        SELECT
            COUNT(id)
        FROM body_part
        WHERE id = NEW.body_part_id
    )
    SELECT
        CASE
            WHEN cnt <= 0
            THEN RAISE(FAIL, 'Невалидный идентификатор ролительского элемента!')
        END
    FROM root_chk;
    WITH
    unq_id_chck(cnt) AS (
        SELECT
            COUNT(body_part_id)
        FROM body_part_nset
        WHERE body_part_id = NEW.body_part_id
    )
    SELECT
        CASE
            WHEN cnt > 0
            THEN RAISE(FAIL, 'Уже есть nset-запись для данной части тела!')
        END
    FROM unq_id_chck;
END
-- ;
