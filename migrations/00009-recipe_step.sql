CREATE TABLE IF NOT EXISTS recipe_step (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	recipe_id INTEGER NOT NULL,
	cooking_action_id INTEGER,
	priority INTEGER DEFAULT 0,
	FOREIGN KEY (recipe_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (cooking_action_id) REFERENCES cooking_action(id) ON DELETE SET NULL
) STRICT
