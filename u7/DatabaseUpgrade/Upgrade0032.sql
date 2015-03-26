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


-------------------------------------------------------------------------------
--
--		is_any_checked_out
--
-------------------------------------------------------------------------------
CREATE FUNCTION is_any_checked_out() RETURNS boolean AS
$BODY$
        SELECT count(*) > 0 FROM CheckOut;
$BODY$
LANGUAGE sql;


-------------------------------------------------------------------------------
--
--		get_last_changeset
--
-------------------------------------------------------------------------------
CREATE FUNCTION get_last_changeset() RETURNS integer AS
$BODY$
        SELECT max(changesetid) FROM Changeset;
$BODY$
LANGUAGE sql;


-------------------------------------------------------------------------------
--
--		create Workstation preset
--
-------------------------------------------------------------------------------

SELECT check_in(1, ARRAY(SELECT id FROM add_file(
            (SELECT "UserID" FROM "User" WHERE "Username"='Administrator'),
            'workstation.hws',
            (SELECT FileID FROM File WHERE Name='HP' AND ParentID=0),
            E'\\x08c5f184e1053a8e0a0a120a100000000000000000000000000000000012200a1e2400280050004100520045004e00540029005f00570053003000300000001a1a0a1857006f0072006b00730074006100740069006f006e000000280032ff044e6574776f726b5c4164617074657230313b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230323b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230333b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230343b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230353b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230363b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230373b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230383b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657230393b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231303b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231313b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231323b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231333b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231343b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231353b737472696e673b303b303b302e302e302e302f33320a4e6574776f726b5c4164617074657231363b737472696e673b303b303b302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723130120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723131120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723032120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723133120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723134120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723033120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723035120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723036120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723135120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723132120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723037120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723034120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723031120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723038120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723136120a302e302e302e302f33323a1f0a114e6574776f726b5c416461707465723039120a302e302e302e302f333280020188020192021a0a185000520045005300450054005f004e0041004d0045000000da06020800')),
    'Upgrade: Create Workstation preset');


-------------------------------------------------------------------------------
--
--		create Monitor preset
--
-------------------------------------------------------------------------------

SELECT check_in(1, ARRAY(SELECT id FROM add_file(
            (SELECT "UserID" FROM "User" WHERE "Username"='Administrator'),
            'monitor.hsw',
            (SELECT FileID FROM File WHERE Name='HP' AND ParentID=0),
            E'\\x088ba8f2810f3a8e020a120a100000000000000000000000000000000012260a242400280050004100520045004e00540029005f004d004f004e00490054004f00520000001a120a104d006f006e00690074006f00720000002800324f4e6574776f726b5c53657276657249505f313b737472696e673b303b303b3132372e302e302e310a4e6574776f726b5c53657276657249505f323b737472696e673b303b303b3132372e302e302e313a1f0a124e6574776f726b5c53657276657249505f3112093132372e302e302e313a1f0a124e6574776f726b5c53657276657249505f3212093132372e302e302e3180020188020192021a0a185000520045005300450054005f004e0041004d0045000000e2060308a846')),
    'Upgrade: Create Monitor preset');


-------------------------------------------------------------------------------
--
--		create DataAcquisitionService preset
--
-------------------------------------------------------------------------------

SELECT check_in(1, ARRAY(SELECT id FROM add_file(
            (SELECT "UserID" FROM "User" WHERE "Username"='Administrator'),
            'dataacquisitionservice.hsw',
            (SELECT FileID FROM File WHERE Name='HP' AND ParentID=0),
            E'\\x088ba8f2810f3ab1020a120a1000000000000000000000000000000000122a0a282400280050004100520045004e00540029005f0044004100530045005200560049004300450000001a300a2e44006100740061004100630071007500690073006900740069006f006e0053006500720076006900630065000000280032504e6574776f726b5c53657276657249505f313b737472696e673b303b303b3132372e302e302e310a4e6574776f726b5c53657276657249505f323b737472696e673b303b303b3132372e302e302e310a3a1f0a124e6574776f726b5c53657276657249505f3112093132372e302e302e313a1f0a124e6574776f726b5c53657276657249505f3212093132372e302e302e3180020188020192021a0a185000520045005300450054005f004e0041004d0045000000e2060308aa46')),
    'Upgrade: Create DataAcquisitionService preset');
