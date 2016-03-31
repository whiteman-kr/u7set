-- MVS		Monitor Visualization Schemas, RPCT-673
--
SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'MVS', 0, '', '{}')), 'Upgrade: MVS system folder was added');

-- Add ProjectProperties Table
--
CREATE TABLE projectproperties
(
	projectpropertiesid	serial PRIMARY KEY,
	name			text	NOT NULL,
	value			text	NOT NULL DEFAULT '',
	UNIQUE(name)
);

------------------------------------------------------------------------------
--
-- Functions: RPCT-640
--
-- set_project_property(property_name text, property_value text);
--
------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION set_project_property(property_name text, property_value text)
RETURNS boolean AS
$BODY$
DECLARE
	result boolean;
BEGIN

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

------------------------------------------------------------------------------
--
-- Functions: RPCT-640
--
-- get_project_property(property_name text);
--
------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_project_property(property_name text)
RETURNS text AS
$BODY$
DECLARE
	result text;
BEGIN
	result := (SELECT Value FROM ProjectProperties WHERE Name = property_name);
	return result;
END;
$BODY$
LANGUAGE plpgsql;



