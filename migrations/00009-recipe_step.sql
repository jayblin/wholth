CREATE TABLE IF NOT EXISTS recipe_step (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	recipe_id INTEGER NOT NULL,
	cooking_action_id INTEGER DEFAULT NULL,
	priority INTEGER DEFAULT 0,
	time INTEGER DEFAULT NULL,
	FOREIGN KEY (recipe_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (cooking_action_id) REFERENCES cooking_action(id) ON DELETE SET NULL
) STRICT;
CREATE INDEX recipe_step_id ON recipe_step (recipe_id)
