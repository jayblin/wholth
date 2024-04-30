CREATE VIEW IF NOT EXISTS
recipe_tree_node(
	lvl,
	recipe_id,
	recipe_mass,
	recipe_ingredient_count,
	ingredient_id,
	ingredient_mass,
	ingredient_weight
) AS
SELECT
	1,
	rs.recipe_id,
	ri.recipe_mass,
	ri.recipe_ingredient_count,
	rsf.food_id,
	rsf.canonical_mass,
	rsf.canonical_mass / ri.recipe_mass
FROM recipe_step rs
INNER JOIN recipe_info ri
	ON ri.recipe_id = rs.recipe_id
INNER JOIN recipe_step_food rsf
	ON rsf.recipe_step_id = rs.id
