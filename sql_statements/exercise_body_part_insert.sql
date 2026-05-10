-- wholth_exec_stmt_Task_INSERT
-- ?1;INT;is_valid_id;Идентификатор упраженения не валиден!
-- ?2;INT;is_valid_id;Идентификатор части тела не валиден!
INSERT INTO exercise_body_part (exercise_id, body_part_id)
VALUES (?1, ?2)
