CREATE TABLE IF NOT EXISTS nutrient_localisation (
	nutrient_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	title TEXT NOT NULL,
	description TEXT,
	FOREIGN KEY (nutrient_id) REFERENCES nutrient(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX unique_locale_nutrient_title ON nutrient_localisation (nutrient_id, locale_id)
