-- wholth_exec_stmt_Task_DELETE
-- ?1;INT;is_valid_id;Невалидный идентифкатор ингредиента!
DELETE FROM recipe_step_food
WHERE
    id = ?1
