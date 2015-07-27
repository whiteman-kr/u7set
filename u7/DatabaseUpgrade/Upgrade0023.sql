CREATE OR REPLACE FUNCTION is_admin(user_id integer)
RETURNS boolean AS
$BODY$
	SELECT
		(Users.Administrator = TRUE AND Users.Disabled = FALSE) AS administrator
	FROM
		Users
	WHERE
		Users.UserID = user_id;
$BODY$
LANGUAGE sql;
