DELETE FROM food_nutrient WHERE nutrient_id = 1008;
DELETE FROM nutrient WHERE id = 1008;
UPDATE nutrient
SET "position" = "position" - 1
WHERE "position" > 10 AND "position" < 20
