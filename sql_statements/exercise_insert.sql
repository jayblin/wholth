-- wholth_exec_stmt_Task_INSERT
-- ?1;TEXT;
-- ?2;TEXT;
-- ?3;INT;is_valid_id;Идентификатор типа упраженения не валиден!
INSERT INTO exercise_modifier_view (
    title,
    description,
    preferred_type_id
)
VALUES (?1, ?2, ?3)
RETURNING (SELECT id FROM exercise WHERE alias = trim(lower(?1)))
