-- wholth_exec_stmt_Task_SELECT
-- ?1;INT;is_valid_id;Невалидный идентифкатор локали!
SELECT
    n.id,
    nlf.title,
    n.unit
FROM nutrient n
INNER JOIN nutrient_localisation nl
    ON n.id = nl.nutrient_id
INNER JOIN nutrient_localisation_fts5 nlf
    ON nlf.rowid = nl.nl_fts5_rowid
WHERE n.position BETWEEN 0 AND 100 AND nl.locale_id = ?1
ORDER BY n.position ASC
