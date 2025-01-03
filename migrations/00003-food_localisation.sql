CREATE TABLE IF NOT EXISTS food_localisation (
	food_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	title TEXT NOT NULL,
	description TEXT,
	FOREIGN KEY (food_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE
    UNIQUE (food_id, locale_id),
    UNIQUE (title COLLATE NOCASE, locale_id)
) STRICT;
CREATE INDEX locale_food_id_search ON food_localisation (food_id)
