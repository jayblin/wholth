CREATE TABLE IF NOT EXISTS migration (
	filename TEXT,
	executed_at TEXT,
	UNIQUE (filename)
) STRICT
