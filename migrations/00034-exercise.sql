CREATE TABLE IF NOT EXISTS exercise_type (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    alias TEXT NOT NULL,
    unit TEXT NOT NULL,
    UNIQUE (alias)
) STRICT;

INSERT INTO exercise_type (id, alias, unit)
VALUES
    (1, 'normal', 'rep'),
    (2, 'explosive', 'rep'),
    (3, 'isometric', 'sec'),
    (4, 'slow_rep', 'rep'),
    (6, 'cardio', 'sec');

CREATE VIRTUAL TABLE IF NOT EXISTS exercise_localisation_fts5 USING fts5(title, description);

CREATE TABLE IF NOT EXISTS exercise (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    alias TEXT NOT NULL,
    preferred_type_id INTEGER DEFAULT NULL,
    el_fts5_rowid INTEGER DEFAULT NULL,
    UNIQUE (alias),
    FOREIGN KEY (preferred_type_id) REFERENCES exercise_type(id)
) STRICT;

CREATE VIEW exercise_modifier_view (
    exercise_id,
    preferred_type_id,
    title,
    description
) AS
SELECT
    e.id,
    e.preferred_type_id,
    ef5.title,
    ef5.description
FROM exercise e
LEFT JOIN exercise_localisation_fts5 ef5
    ON ef5.rowid = e.el_fts5_rowid;

CREATE VIEW exercise_validator_view (
    action,
    exercise_id,
    preferred_type_id,
    title,
    description
) AS
SELECT NULL, ev.* FROM exercise_modifier_view ev;

CREATE TRIGGER IF NOT EXISTS exercise_validator_view_trigger_insert
INSTEAD OF INSERT ON exercise_validator_view
BEGIN
    SELECT CASE
        WHEN NEW.preferred_type_id IS NULL
        THEN true
        WHEN NEW.preferred_type_id = '' OR typeof(NEW.preferred_type_id) <> 'integer'
        THEN RAISE(FAIL, 'Идентификатор типа упраженения не валиден!')
        WHEN 0 = (SELECT COUNT(id) FROM exercise_type WHERE id = NEW.preferred_type_id)
        THEN RAISE(FAIL, 'Не удалось найти тип упражнения с таким идентификатором!')
    END;

    WITH title_chk(_title) AS (
        SELECT * FROM (VALUES (lower(trim(NEW.title))))
    )
    SELECT
        CASE
            WHEN NEW.action = 'update' AND _title IS NULL
            THEN true
            WHEN _title = ''
            THEN RAISE(FAIL, 'Для упражнения нужно заполнить название!')
            WHEN NEW.action <> 'update' AND 0 < (SELECT COUNT(id) FROM exercise WHERE alias = _title)
            THEN RAISE(FAIL, 'Уже есть упражнение с таким названием!')
        END
    FROM title_chk;
END;

CREATE TRIGGER IF NOT EXISTS exercise_modifier_view_trigger_insert
INSTEAD OF INSERT ON exercise_modifier_view
BEGIN
    INSERT INTO exercise_validator_view (
        action,
        preferred_type_id,
        title,
        description
    ) VALUES (
        'insert',
        NEW.preferred_type_id,
        NEW.title,
        NEW.description
    );

    INSERT INTO exercise_localisation_fts5 (title, description)
    VALUES (lower(trim(NEW.title)), NEW.description);

    INSERT INTO exercise (alias, preferred_type_id, el_fts5_rowid)
    VALUES
    (
        lower(trim(NEW.title)),
        NEW.preferred_type_id,
        last_insert_rowid()
    );
END;

CREATE TRIGGER IF NOT EXISTS exercise_modifier_view_trigger_update
INSTEAD OF UPDATE ON exercise_modifier_view
BEGIN
    SELECT CASE
        WHEN 1 <> (SELECT COUNT(id) FROM exercise WHERE id = NEW.exercise_id)
        THEN RAISE(FAIL, 'Не существует упражнения с таким идентификатором!')
    END;

    INSERT INTO exercise_validator_view (
        action,
        preferred_type_id,
        title,
        description
    ) VALUES (
        'update',
        NEW.preferred_type_id,
        NEW.title,
        NEW.description
    );

    UPDATE exercise_localisation_fts5
    SET title = coalesce(lower(trim(NEW.title)), title),
        description = coalesce(NEW.description, description)
    WHERE rowid = (SELECT el_fts5_rowid FROM exercise WHERE id = NEW.exercise_id);

    UPDATE exercise
    SET alias = coalesce(lower(trim(NEW.title)), alias),
        preferred_type_id = coalesce(NEW.preferred_type_id, preferred_type_id);
END
