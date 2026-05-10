CREATE TABLE IF NOT EXISTS exercise_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    exercise_id INTEGER NOT NULL,
    type_id INTEGER NOT NULL,
    value INT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (exercise_id) REFERENCES exercise(id),
    FOREIGN KEY (type_id) REFERENCES exercise_type(id)
) STRICT;

CREATE VIEW exercise_log_validator_view (
    id,
    exercise_id,
    type_id,
    value,
    created_at
) AS
SELECT 
    el.id,
    el.exercise_id,
    el.type_id,
    el.value,
    el.created_at
FROM exercise_log el;

CREATE TRIGGER IF NOT EXISTS exercise_log_view_trigger_insert
INSTEAD OF INSERT ON exercise_log_validator_view
BEGIN
    SELECT CASE
        WHEN NEW.exercise_id IS NULL
        THEN RAISE(FAIL, 'Не передан идентификатор упражнения!')
        WHEN 0 = (SELECT COUNT(id) FROM exercise WHERE id = NEW.exercise_id)
        THEN RAISE(FAIL, 'Не удалось найти упражнение с таким идентификатором!')
    END;

    SELECT CASE
        WHEN NEW.type_id IS NULL
        THEN TRUE
        WHEN NEW.type_id = '' OR typeof(NEW.type_id) <> 'integer'
        THEN RAISE(FAIL, 'Идентификатор типа упраженения не валиден!')
        WHEN 0 = (SELECT COUNT(id) FROM exercise_type WHERE id = NEW.type_id)
        THEN RAISE(FAIL, 'Не удалось найти тип упражнения с таким идентификатором!')
    END;

    SELECT CASE
        WHEN NEW.value < 0
        THEN RAISE(FAIL, 'Значение не может быть меньше 0!')
    END;

    SELECT CASE
        WHEN NEW.created_at IS NULL
        THEN true
        WHEN datetime(NEW.created_at) IS NULL
        THEN RAISE(FAIL, 'Невалидная дата/время!')
    END;
END;

CREATE TRIGGER IF NOT EXISTS exercise_log_trigger_before_insert
BEFORE INSERT ON exercise_log
BEGIN
    INSERT INTO exercise_log_validator_view (exercise_id, type_id, value, created_at)
    VALUES (NEW.exercise_id, NEW.type_id, NEW.value, NEW.created_at);
END;

CREATE TRIGGER IF NOT EXISTS exercise_log_trigger_before_update
BEFORE UPDATE ON exercise_log
BEGIN
    INSERT INTO exercise_log_validator_view (exercise_id, type_id, value, created_at)
    VALUES (NEW.exercise_id, NEW.type_id, NEW.value, NEW.created_at);
END
