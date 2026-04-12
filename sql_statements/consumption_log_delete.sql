-- wholth_exec_stmt_Task_DELETE
-- ?1;INT;is_valid_id;Невалидный идентифкатор лога!
-- ?2;INT;is_valid_id;Невалидный идентифкатор пользователя!
DELETE FROM consumption_log
WHERE id = ?1 AND user_id = ?2
