CREATE TABLE IF NOT EXISTS cooking_action_localisation (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	cooking_action_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	title TEXT NOT NULL,
	description TEXT,
	FOREIGN KEY (cooking_action_id) REFERENCES cooking_action(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE
) STRICT
