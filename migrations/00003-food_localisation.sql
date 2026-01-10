CREATE VIRTUAL TABLE food_localisation_fts5 USING fts5(title, description);
CREATE TABLE IF NOT EXISTS food_localisation (
	food_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	fl_fts5_rowid INTEGER DEFAULT NULL,
	FOREIGN KEY (food_id) REFERENCES food(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE,
    UNIQUE (food_id, locale_id)
) STRICT;
CREATE INDEX locale_food_id_search ON food_localisation (food_id)
