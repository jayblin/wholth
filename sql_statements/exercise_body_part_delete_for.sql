-- wholth_exec_stmt_Task_DELETE
-- ?1;INT;is_valid_id;Идентификатор упраженения не валиден!
DELETE FROM exercise_body_part WHERE exercise_id = ?1
