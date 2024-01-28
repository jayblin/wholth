CREATE TABLE IF NOT EXISTS food_nutrient (
	food_id INTEGER NOT NULL,
	nutrient_id INTEGER NOT NULL,
	value REAL,
	created_at TEXT NOT NULL,
	FOREIGN KEY (food_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (nutrient_id) REFERENCES nutrient(id) ON DELETE CASCADE
) STRICT;
CREATE UNIQUE INDEX unique_food_nutrient_ids ON food_nutrient (food_id, nutrient_id);
CREATE INDEX food_nutrient_id_search ON food_nutrient (food_id)
