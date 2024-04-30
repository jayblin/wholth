CREATE TABLE IF NOT EXISTS error_localisation (
	error_id INTEGER NOT NULL,
	locale_id INTEGER NOT NULL,
	description TEXT NOT NULL,
	FOREIGN KEY (error_id) REFERENCES error(id) ON DELETE CASCADE,
	FOREIGN KEY (locale_id) REFERENCES locale(id) ON DELETE CASCADE
)
