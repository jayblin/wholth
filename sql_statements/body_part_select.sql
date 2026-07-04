-- wholth_exec_stmt_Task_SELECT
SELECT
    bp.id,
    bpl.title,
    bpl.description,
    bpn.lft,
    bpn.rgt
FROM body_part bp
LEFT JOIN body_part_localisation_fts5 bpl
    ON bpl.rowid = bp.bpl_fts5_rowid
LEFT JOIN body_part_nset bpn
    ON bpn.body_part_id = bp.id
ORDER BY bpn.lft ASC
LIMIT 1000
