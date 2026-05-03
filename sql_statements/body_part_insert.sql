-- wholth_exec_stmt_Task_INSERT
-- ?1;TEXT;
-- ?2;TEXT;
INSERT INTO body_part_localisation_fts5 (title, description)
VALUES (lower(trim(coalesce(?1, ''))), coalesce(?2, ''));

INSERT INTO body_part (bpl_fts5_rowid, unique_title)
SELECT rowid, title
FROM body_part_localisation_fts5
WHERE rowid = last_insert_rowid();

-- ?1;INT;is_valid_id;Невалидный идентификатор ролительского элемента!
WITH _data AS (
    SELECT
        body_part_id AS parent_id,
        last_insert_rowid() AS body_part_id,
        rgt AS lft,
        rgt + 1 AS rgt
    FROM body_part_nset
    WHERE body_part_id = coalesce(?1, 1)
    UNION ALL
    SELECT * FROM (VALUES (0,0,0,0)) AS _a
)
INSERT INTO body_part_nset (body_part_id, lft, rgt)
SELECT _d.body_part_id, _d.lft, _d.rgt
FROM (SELECT * FROM _data ORDER BY parent_id DESC LIMIT 1) AS _d;

UPDATE body_part_nset
SET
   rgt = rgt+2
WHERE rgt > (
    SELECT rgt
    FROM body_part_nset
    WHERE body_part_id = last_insert_rowid())
;

UPDATE body_part_nset
SET
   rgt = rgt+2
WHERE rgt = (
    SELECT lft
    FROM body_part_nset
    WHERE body_part_id = last_insert_rowid())
;

UPDATE body_part_nset
SET
   lft = lft+2
WHERE lft >= (
    SELECT rgt
    FROM body_part_nset
    WHERE body_part_id = last_insert_rowid())
;

SELECT body_part_id FROM body_part_nset WHERE rowid = last_insert_rowid()
