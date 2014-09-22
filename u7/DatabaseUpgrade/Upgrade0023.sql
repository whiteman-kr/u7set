CREATE OR REPLACE FUNCTION is_admin(user_id integer)
RETURNS boolean AS
$BODY$
	SELECT
		("Administrator" = TRUE AND "Disabled" = FALSE) AS administrator
	FROM
		"User"
	WHERE
		"UserID" = user_id;
$BODY$
LANGUAGE sql;
