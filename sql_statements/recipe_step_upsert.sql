-- wholth_exec_stmt_Task_INSERT
-- ?1;INT;is_valid_id;Невалидный идентифкатор рецепта(пищи)!
-- ?2;INT;
INSERT INTO recipe_step (recipe_id, seconds)
VALUES (?1, ?2)
ON CONFLICT(recipe_id) DO UPDATE
    SET seconds = COALESCE(?2, seconds);
-- ?1;INT;
-- ?2;INT;is_valid_id;Невалидный идентифкатор локали!
-- ?3;TEXT;
INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, description)
    SELECT id, ?2, COALESCE(?3, description)
    FROM recipe_step
    LEFT JOIN recipe_step_localisation
        ON recipe_step_id = id
    WHERE recipe_id = ?1
ON CONFLICT(recipe_step_id, locale_id) DO UPDATE
    SET description = COALESCE(?3, description)
RETURNING recipe_step_id
