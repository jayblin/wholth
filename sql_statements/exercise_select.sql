-- wholth_exec_stmt_Task_SELECT
-- ?1;INT;
-- ?2;TEXT;
-- ?3;INT;
-- ?4;INT;
WITH
text_search AS NOT MATERIALIZED (
    SELECT
        el.rowid as rowid,
        el.title as title,
        el.description as description
    FROM exercise_localisation_fts5 el
    WHERE exercise_localisation_fts5 MATCH ?2
),
the_list AS (
    SELECT
        e.id as id,
        el.title as title,
        el.description as description,
        e.preferred_type_id as preferred_type_id,
        et.alias as preferred_type_alias,
        et.unit as preferred_type_unit,
        group_concat(ebp.body_part_id, ',') as body_part_ids
    FROM exercise e
    LEFT JOIN exercise_localisation_fts5 el
        ON el.rowid = e.el_fts5_rowid
    LEFT JOIN exercise_type et
        ON et.id = e.preferred_type_id
    LEFT JOIN exercise_body_part ebp
        ON ebp.exercise_id = e.id
    WHERE
        CASE
            WHEN ?1 IS NOT NULL
            THEN e.id = ?1
            WHEN ?2 IS NOT NULL
            THEN (EXISTS (SELECT rowid FROM text_search WHERE rowid = e.el_fts5_rowid))
            ELSE TRUE
        END
    GROUP BY e.id
    -- ORDER BY WHEN ?4 IS NOT NULL THEN END END
    ORDER BY title DESC NULLS LAST
)
SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL, NULL, NULL FROM the_list
UNION ALL
SELECT * FROM (
    SELECT *
    FROM the_list
    LIMIT CASE WHEN ?3 IS NOT NULL THEN ?3 ELSE 100 END
    OFFSET CASE WHEN ?4 IS NOT NULL THEN ?4 ELSE 0 END
)
