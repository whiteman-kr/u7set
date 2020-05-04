CREATE OR REPLACE FUNCTION api.get_user_property_list(
	session_key text,
	property_template text)
RETURNS SETOF text AS
$BODY$
DECLARE
	user_id integer;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);

	return QUERY (SELECT Name FROM UserProperties WHERE UserID = user_id AND Name LIKE property_template ORDER BY Name);
END;
$BODY$
LANGUAGE plpgsql;