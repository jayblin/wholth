-- wholth_exec_stmt_Task_INSERT
-- temporary storage cannot be changed from within a transaction in "PRAGMA temp_store = MEMORY;
-- PRAGMA temp_store = MEMORY;
CREATE TEMP TABLE _vars(id INTEGER, title TEXT, description TEXT) STRICT;
-- ?1;INT;
-- ?2;TEXT;
-- ?3;TEXT;
INSERT INTO _vars
VALUES (?1, ?2, ?3);

UPDATE body_part
SET
    unique_title = CASE
        WHEN _vars.title IS NULL
        THEN unique_title
        ELSE lower(trim(_vars.title))
    END
FROM _vars
WHERE body_part.id = _vars.id 
;

UPDATE body_part_localisation_fts5
SET
    title = bp.unique_title,
    description = CASE
        WHEN _vars.description IS NULL
        THEN body_part_localisation_fts5.description
        ELSE _vars.description
    END
FROM body_part bp
JOIN _vars ON 1=1
WHERE rowid = bp.bpl_fts5_rowid AND _vars.id = bp.id
