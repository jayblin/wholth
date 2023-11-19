CREATE TABLE IF NOT EXISTS food_localisation (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	food_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	title TEXT NOT NULL,
	description TEXT,
	FOREIGN KEY (food_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE
) STRICT;
CREATE UNIQUE INDEX unique_locale_food_title ON food_localisation (title, locale_id);
CREATE INDEX locale_food_id_search ON food_localisation (food_id)
