CREATE OR REPLACE FUNCTION get_user_id (IN user_name text, IN user_password text) RETURNS integer AS
$BODY$
	SELECT Users.UserID FROM Users
		WHERE Users.Username = user_name AND Users.Password = user_password;
$BODY$
LANGUAGE sql;

CREATE OR REPLACE FUNCTION check_user_password(IN user_name text, IN user_password text) RETURNS boolean AS
$BODY$
	SELECT COUNT(Users.UserID) > 0 FROM Users
		WHERE Users.Username = user_name AND Users.Password = user_password;
$BODY$
LANGUAGE sql;

CREATE OR REPLACE FUNCTION check_user_password(IN user_id integer, IN user_password text) RETURNS boolean AS
$BODY$
	SELECT COUNT(Users.UserID) > 0 FROM Users
		WHERE Users.UserID = user_id AND Users.Password = user_password;
$BODY$
LANGUAGE sql;
