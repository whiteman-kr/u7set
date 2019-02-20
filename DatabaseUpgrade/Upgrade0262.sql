-- RPCT-1846 - Add option to uppercase AppSignalID
--

DROP FUNCTION public.set_project_property(text, text);

CREATE OR REPLACE FUNCTION api.set_project_property(
	session_key text,
    property_name text,
    property_value text)
  RETURNS boolean AS
$BODY$
DECLARE
	result boolean;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	IF (property_name IS NULL OR char_length(property_name) = 0)
	THEN
		return FALSE;
	END IF;

	INSERT INTO ProjectProperties(Name, Value)
		VALUES (property_name, property_value)
		ON CONFLICT (Name) DO
	UPDATE SET Value = property_value;

	result = (SELECT (Name = property_name)
		FROM ProjectProperties WHERE ProjectProperties.Name = property_name);

	IF (result is NULL) THEN
		result = FALSE;
	END IF;

	return result;
END;
$BODY$
LANGUAGE plpgsql;



DROP FUNCTION public.get_project_property(text);

CREATE OR REPLACE FUNCTION api.get_project_property(
	session_key text,
	property_name text)
  RETURNS text AS
$BODY$
DECLARE
	result text;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	result := (SELECT Value FROM ProjectProperties WHERE Name = property_name);
	return result;
END;
$BODY$
LANGUAGE plpgsql;



SELECT * FROM api.set_project_property('$(SessionKey)', 'UppercaseAppSignalID', 'false');