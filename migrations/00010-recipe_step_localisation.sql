CREATE TABLE IF NOT EXISTS recipe_step_localisation (
	recipe_step_id INTEGER NOT NULL,
	description TEXT,
	FOREIGN KEY (recipe_step_id) REFERENCES recipe_step(id) ON DELETE CASCADE
) STRICT
