-- wholth_exec_stmt_Task_SELECT
-- ?1;INT;
-- ?2;TEXT;
-- ?3;INT;
-- ?4;TEXT;
SELECT
    e.id as id,
    e.alias as alias,
    el.title as title,
    el.description as description,
    e.preferred_type_id as preferred_type_id,
    et.alias as preferred_type_alias,
    et.unit as preferred_type_unit,
FROM exercise e
LEFT JOIN exercise_localisation_fts5 el
    ON el.rowid = e.el_fts5_rowid
LEFT JOIN exercise_type et
    ON et.id = e.preferred_type_id
WHERE
    (?1 IS NOT NULL AND e.id = ?1)
    OR (?2 IS NOT NULL AND exercise_localisation_fts5 MATCH '{title}:' || ?2)
-- ORDER BY WHEN ?4 IS NOT NULL THEN END END
ORDER title DESC NULLS LAST
LIMIT CASE WHEN ?3 IS NOT NULL THEN ?3 ELSE 100 END
