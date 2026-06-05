-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Невалидный идентифкатор лога!
-- ?2;INT;is_valid_id;Невалидный идентифкатор пользователя!
-- ?3;INT;
-- ?4;TEXT;
UPDATE consumption_log SET
    mass = coalesce(?3, mass),
    consumed_at = coalesce(?4, consumed_at)
WHERE id = ?1 AND user_id = ?2
