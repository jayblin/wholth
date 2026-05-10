-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Идентификатор лога не валиден!
-- ?2;INT;is_valid_id;Идентификатор типа упраженения не валиден!
-- ?3;INT;
-- ?4;TEXT;
UPDATE exercise_log
SET
    type_id = coalesce(?2, type_id),
    value = coalesce(?3, value),
    created_at = coalesce(?4, created_at)
WHERE id = ?1
