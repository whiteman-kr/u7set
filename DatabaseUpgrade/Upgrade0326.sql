-- Add UserProperties Table
--
CREATE TABLE userproperties
(
	userpropertiesid	serial  PRIMARY KEY,
	userid 				integer NOT NULL REFERENCES users,
	name				text	NOT NULL,
	value				text	NOT NULL DEFAULT '',
	UNIQUE(userid, name)
);


CREATE OR REPLACE FUNCTION api.set_user_property(
	session_key text,
    property_name text,
    property_value text)
RETURNS boolean AS
$BODY$
DECLARE
    user_id integer;
	result boolean;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	IF (property_name IS NULL OR char_length(property_name) = 0)
	THEN
		return FALSE;
	END IF;

	user_id := user_api.current_user_id(session_key);

	INSERT INTO UserProperties(UserID, Name, Value)
		VALUES (user_id, property_name, property_value)
		ON CONFLICT (UserID, Name) DO
	UPDATE SET Value = property_value;

	result = (SELECT EXISTS( 
		SELECT 1
		FROM 
			UserProperties
		WHERE 
			UserProperties.UserID = user_id AND 
			UserProperties.Name = property_name AND 
			UserProperties.Value = property_value));

	return result;
END;
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_user_property(
	session_key text,
	property_name text)
RETURNS text AS
$BODY$
DECLARE
	user_id integer;
	result text;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);

	result := (SELECT Value FROM UserProperties WHERE UserID = user_id AND Name = property_name);
	return result;
END;
$BODY$
LANGUAGE plpgsql;
