-------------------------------------------------------------------------------
--
--		Add Module Configuration "MC" file, and move configuration files to MC
--
-------------------------------------------------------------------------------

-- Add System folders (files) for different types
-- MC		Module Configuration


SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'MC', 0, '')), 'Upgrade: MC system folder was added');

-- These files are not used anymore, but we will kepp them, just in case
UPDATE File SET ParentID = (SELECT FileID FROM File WHERE Name = 'MC' AND ParentID = 0 AND Deleted = FALSE)
	WHERE Name LIKE '%.cdf' OR Name LIKE '%.cdb' OR Name LIKE '%.cdd';

-- Create new ModulesConfigurations.descr file where all configurations descriptions  (for all modules)
-- and scripts will be kept
--
SELECT add_file(
	(SELECT "UserID" FROM "User" WHERE "Username"='Administrator'),
	'ModulesConfigurations.descr',
	(SELECT FileID FROM File WHERE Name='MC' AND ParentID=0),
	'');


-------------------------------------------------------------------------------
--
--		create_user
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION
	create_user(
		current_user_id integer,
		user_name text,
		first_name text,
		last_name text,
		user_password text,
		is_admin boolean,
		is_read_only boolean,
		is_disabled boolean)
RETURNS integer AS
$BODY$
DECLARE
	new_user_id integer;
BEGIN

	IF (is_admin(current_user_id) = FALSE OR is_admin(current_user_id) IS NULL) THEN
		RAISE 'User % has no right to add another user.', current_user_id;
	END IF;

	IF (SELECT count(*) FROM "User" WHERE "Username" = user_name) > 0 THEN
		RAISE 'User % already exists.', user_name;
	END IF;

	INSERT INTO "User" ("Username", "FirstName", "LastName", "Password", "Administrator", "ReadOnly", "Disabled")
		VALUES (user_name, first_name, last_name, user_password, is_admin, is_read_only, is_disabled) RETURNING "UserID" INTO new_user_id;

	RETURN new_user_id;
END
$BODY$
LANGUAGE plpgsql;
