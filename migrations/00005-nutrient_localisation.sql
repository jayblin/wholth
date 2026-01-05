CREATE VIRTUAL TABLE nutrient_localisation_fts5 USING fts5(title, description);
CREATE TABLE IF NOT EXISTS nutrient_localisation (
	nutrient_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	nl_fts5_rowid INTEGER NOT NULL,
	FOREIGN KEY (nutrient_id) REFERENCES nutrient(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE,
    UNIQUE (nutrient_id, locale_id)
)
