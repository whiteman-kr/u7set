CREATE OR REPLACE FUNCTION api.remove_user_property(
	session_key text,
	property_name text)
RETURNS boolean AS
$BODY$
DECLARE
	user_id integer;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);
	DELETE FROM UserProperties WHERE UserID = user_id AND Name = property_name;
	
	RETURN TRUE;
END;
$BODY$
LANGUAGE plpgsql;