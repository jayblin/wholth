-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Невалидный идентифкатор лога!
-- ?2;INT;is_valid_id;Невалидный идентифкатор пользователя!
-- ?3;INT;
UPDATE consumption_log SET mass = ?3
WHERE id = ?1 AND user_id = ?2
