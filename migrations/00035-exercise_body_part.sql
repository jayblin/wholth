CREATE TABLE IF NOT EXISTS exercise_body_part (
    exercise_id INTEGER NOT NULL,
    body_part_id INTEGER NOT NULL,
    FOREIGN KEY (exercise_id) REFERENCES exercise(id),
    FOREIGN KEY (body_part_id) REFERENCES body_part(id),
    UNIQUE (exercise_id, body_part_id)
) STRICT;

CREATE TRIGGER IF NOT EXISTS exercise_body_part_trigger_before_insert
BEFORE INSERT ON exercise_body_part
BEGIN
    SELECT CASE
        WHEN NEW.exercise_id IS NULL
        THEN RAISE(FAIL, 'Не передан идентификатор упражнения!')
        WHEN 0 = (SELECT COUNT(id) FROM exercise WHERE id = NEW.exercise_id)
        THEN RAISE(FAIL, 'Не удалось найти упражнение с таким идентификатором!')
    END;

    SELECT CASE
        WHEN NEW.body_part_id IS NULL
        THEN RAISE(FAIL, 'Не передан идентификатор части тела!')
        WHEN 0 = (SELECT COUNT(id) FROM body_part WHERE id = NEW.body_part_id)
        THEN RAISE(FAIL, 'Не удалось найти часть тела с таким идентификатором!')
    END;

    SELECT CASE
        WHEN 1 = (
            SELECT 1 FROM exercise_body_part
            WHERE body_part_id = NEW.body_part_id AND exercise_id = NEW.exercise_id
        )
        THEN RAISE(FAIL, 'Упражнение уже привязано к части тела!')
    END;
END
