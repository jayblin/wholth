CREATE TABLE IF NOT EXISTS recipe_step_food (
	recipe_step_id INTEGER NOT NULL,
	food_id INTEGER,
	canonical_mass REAL,
	FOREIGN KEY (recipe_step_id) REFERENCES recipe_step(id) ON DELETE CASCADE,
	FOREIGN KEY (food_id) REFERENCES food(id) ON DELETE CASCADE
) STRICT;
CREATE INDEX recipe_step_food_recipe_step_id ON recipe_step_food (recipe_step_id)
