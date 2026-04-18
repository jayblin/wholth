-- wholth_exec_stmt_Task_INSERT
-- ?1;TEXT;
-- ?2;TEXT;
INSERT INTO body_part_localisation_fts5 (title, description)
VALUES (COALESCE(?1, 'N/A'), COALESCE(?2, 'N/A'));

INSERT INTO body_part (bpl_fts5_rowid)
VALUES (last_insert_rowid());

-- ?1;INT;is_valid_id;Невалидный идентификатор ролительского элемента
WITH _data AS (
    SELECT
        rgt AS lft,
        rgt + 1 AS rgt
    FROM body_part_nset
    WHERE body_part_id = ?1
)
INSERT INTO body_part_nset (body_part_id, lft, rgt)
SELECT last_insert_rowid(), _data.lft, _data.rgt FROM _data;

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
