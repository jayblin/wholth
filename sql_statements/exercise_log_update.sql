-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Идентификатор лога не валиден!
-- ?2;INT;is_valid_id;Идентификатор пользователя не валиден!
-- ?3;INT;is_valid_id;Идентификатор типа упраженения не валиден!
-- ?4;INT;
-- ?5;TEXT;
UPDATE exercise_log
SET
    type_id = coalesce(?3, type_id),
    value = coalesce(?4, value),
    created_at = coalesce(?5, created_at)
WHERE id = ?1 AND user_id = ?2
