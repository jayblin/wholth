CREATE TABLE IF NOT EXISTS food (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	created_at TEXT NOT NULL,
	calories REAL,
	proteins REAL,
	carbohydrates REAL,
	fats REAL
) STRICT
