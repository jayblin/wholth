-- wholth_exec_stmt_Task_SELECT
-- ?1;INT;
-- ?2;TEXT;
-- ?3;TEXT;
-- ?4;TEXT;
-- ?5;TEXT;
-- ?6;INT;
-- ?7;INT;
WITH
text_search AS NOT MATERIALIZED (
    SELECT
        el.rowid as rowid,
        el.title as title,
        el.description as description
    FROM exercise_localisation_fts5 el
    WHERE exercise_localisation_fts5 MATCH ?3
),
the_list AS (
    SELECT
        el.id as id,
        el.value as value,
        el.created_at as created_at,
        -- exercise
        e.id as exercise_id,
        elf.title as exercise_title,
        -- type
        et.id as type_id,
        et.alias as type_alias,
        et.unit as type_unit
    FROM exercise_log el
    INNER JOIN exercise e
        ON e.id = el.exercise_id
    LEFT JOIN exercise_localisation_fts5 elf
        ON elf.rowid = e.el_fts5_rowid
    LEFT JOIN exercise_type et
        ON et.id = el.type_id
    LEFT JOIN exercise_body_part ebp
        ON ebp.exercise_id = e.id
    WHERE
        el.user_id = ?1
        AND CASE
            WHEN ?2 IS NOT NULL
            THEN el.id = ?2
            WHEN ?3 IS NOT NULL
            THEN (EXISTS (SELECT rowid FROM text_search WHERE rowid = e.el_fts5_rowid))
            ELSE TRUE
        END
        AND CASE
            WHEN ?4 IS NOT NULL AND ?5 IS NOT NULL
            THEN created_at BETWEEN ?4 AND ?5
            ELSE TRUE
        END
    GROUP BY el.id
    ORDER BY created_at DESC NULLS LAST
)
SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL, NULL, NULL, NULL FROM the_list
UNION ALL
SELECT * FROM (
    SELECT *
    FROM the_list
    LIMIT CASE WHEN ?6 IS NOT NULL THEN ?6 ELSE 100 END
    OFFSET CASE
        WHEN ?7 IS NOT NULL
        THEN
            CASE
                WHEN ?6 IS NOT NULL
                THEN ?6 * ?7
                ELSE 100 * ?7
            END
        ELSE 0
    END
)
