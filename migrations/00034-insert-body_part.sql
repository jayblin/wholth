INSERT INTO body_part_localisation_fts5 (rowid, title) VALUES
(1,'тело'),
(2,'шея'),
(3,'руки'),
(4,'спина верх'),
(5,'спина низ'),
(6,'ноги')
;
INSERT INTO body_part (id, bpl_fts5_rowid) VALUES
(1,1),
(2,2),
(3,3),
(4,4),
(5,5),
(6,6)
;
INSERT INTO body_part_nset (body_part_id, lft, rgt) VALUES
(1,1,12),
(2,2,3),
(3,4,5),
(4,6,7),
(5,8,9),
(6,10,11)
