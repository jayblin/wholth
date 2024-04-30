CREATE VIEW IF NOT EXISTS
recipe_info(
	recipe_id,
	recipe_mass,
	recipe_ingredient_count
) AS
SELECT
	rs.recipe_id,
	SUM(rsf.canonical_mass),
	COUNT(rsf.food_id)
FROM recipe_step rs
INNER JOIN recipe_step_food rsf
	ON rsf.recipe_step_id = rs.id
GROUP BY rs.recipe_id
