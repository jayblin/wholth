-- wholth_exec_stmt_Task_UPDATE
-- ?1;INT;is_valid_id;Идентификатор лога не валиден!
-- ?2;INT;is_valid_id;Идентификатор пользователя не валиден!
DELETE FROM exercise_log
WHERE id = ?1 and user_id = ?2
