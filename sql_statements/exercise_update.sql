-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Идентификатор упраженения не валиден!
-- ?2;TEXT;
-- ?3;TEXT;
-- ?4;INT;is_valid_id;Идентификатор типа упраженения не валиден!
UPDATE exercise_modifier_view
SET
    exercise_id = ?1,
    title = ?2,
    description = ?3,
    preferred_type_id = ?4
