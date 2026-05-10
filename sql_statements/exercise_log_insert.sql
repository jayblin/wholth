-- wholth_exec_stmt_Task_INSERT
-- ?1;INT;is_valid_id;Идентификатор упраженения не валиден!
-- ?2;INT;is_valid_id;Идентификатор типа упраженения не валиден!
-- ?3;INT;
-- ?4;TEXT;
INSERT INTO exercise_log (exercise_id, type_id, value, created_at)
VALUES (?1, ?2, ?3, coalesce(?4, CURRENT_TIMESTAMP))
RETURNING id
