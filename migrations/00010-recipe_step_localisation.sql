CREATE TABLE IF NOT EXISTS recipe_step_localisation (
	recipe_step_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	description TEXT,
	FOREIGN KEY (recipe_step_id) REFERENCES recipe_step(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE,
    UNIQUE (recipe_step_id, locale_id)
) STRICT;
CREATE INDEX recipe_step_localisation_recipe_step_id ON recipe_step_localisation (recipe_step_id)
