CREATE VIRTUAL TABLE IF NOT EXISTS body_part_localisation_fts5 USING fts5(title, description);

CREATE TABLE IF NOT EXISTS body_part (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	bpl_fts5_rowid INTEGER NOT NULL,
    UNIQUE (bpl_fts5_rowid)
) STRICT;

CREATE TABLE IF NOT EXISTS body_part_nset (
	body_part_id INTEGER,
    lft INTEGER NOT NULL,
    rgt INTEGER NOT NULL,
	FOREIGN KEY (body_part_id) REFERENCES body_part(id)
) STRICT
-- ;
-- 
-- CREATE TRIGGER IF NOT EXISTS update_nset_trigger AFTER INSERT ON body_part_nset
-- BEGIN
--     UPDATE body_part_nset
--     SET
--         rgt = rgt+1
--     WHERE rgt >= (SELECT lft FROM body_part_nset WHERE body_part_id = last_insert_rowid())
--         AND body_part_id <> last_insert_rowid();
-- END
