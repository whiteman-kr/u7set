CREATE FUNCTION "GetUserID" (IN username text, IN password text) RETURNS integer AS
$BODY$
    SELECT "UserID" FROM "User"
        WHERE "Username"=username AND "Password"=password;
$BODY$
LANGUAGE sql;
