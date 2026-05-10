-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Идентификатор лога не валиден!
DELETE FROM exercise_log
WHERE id = ?1
