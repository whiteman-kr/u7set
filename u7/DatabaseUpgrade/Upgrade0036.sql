------------------------------------------------------------------------------
--
-- Functions:
--
-- get_file_id(user_id integer, parent_id integer, file_name text)
-- get_file_id(user_id integer, full_file_name text)
-- add_or_update_file(user_id integer, full_parent_file_name text, file_name text, checkin_comment text, file_data bytea)
-- is_admin (RPCT-93) - fixed: function can return NULL fro non existing user
--
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION get_file_id(user_id integer, parent_id integer, file_name text)
RETURNS integer AS
$BODY$
DECLARE
	found_file_id integer;
	just_created boolean;
	owner_id int;
BEGIN
	-- find file
	found_file_id := (SELECT F.FileID 
				FROM File F 
				WHERE F.ParentId = parent_id AND 
					F.Name = file_name AND
					F.Deleted = FALSE);

	IF (found_file_id IS NULL) THEN
		RAISE EXCEPTION 'File % not found', file_name;
	END IF;

	-- If user an admin, he or she can see this file
	IF (is_admin(user_id) = TRUE) THEN
		RETURN found_file_id;
	END IF;

	-- if file was created and was not checked in yet, 
	-- file is exists only for the user who created it, 
	-- as it can be totaly removed by undo operation
	just_created := (SELECT COUNT(*) > 0 FROM File F WHERE F.FileId = found_file_id AND F.CheckedInInstanceId IS NULL);

	IF (just_created = FALSE) THEN
		RETURN found_file_id;
	END IF;
	
	-- Get owner of the file and compare it to user_id
	owner_id := (SELECT UserId FROM CheckOut CO WHERE CO.FileID = found_file_id);

	IF (owner_id <> user_id) THEN
		RAISE EXCEPTION 'File % not found, it was created by another user.', file_name;
	END IF;
		
	RETURN found_file_id;
END
$BODY$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_file_id(user_id integer, full_file_name text)
RETURNS integer AS
$BODY$
DECLARE
	split_names text[];
	fn text;
	found_file_id integer;
	just_created boolean;
	owner_id int;
BEGIN
	-- user_id -- who performs query
	-- full_file_name -- file with full path, example '$root$/MC/ModuleConfigration.jscript'
	-- EXAMPLE: SELECT * FROM get_file_id(1, '///$root$/HC/device-cc6cd30a-defb-11e4-bdb2-5238966c092f.hsm///');

	-- Find file
	full_file_name := trim(both '/' from full_file_name);   -- remove accident extra /

	split_names = regexp_split_to_array(full_file_name, E'\\/{1}'); -- split path on files 

	found_file_id := NULL;  -- $root$ has NULL as ParentId

	FOREACH fn IN ARRAY split_names
	LOOP
		IF (found_file_id IS NULL) THEN   -- Condion is required to resolve IS NULL staff
			found_file_id := (SELECT FileID FROM File WHERE Name = fn AND ParentID IS NULL AND Deleted = FALSE);
		ELSE
			found_file_id := (SELECT FileID FROM File WHERE Name = fn AND ParentID = found_file_id AND Deleted = FALSE);
		END IF;

		IF (found_file_id IS NULL) THEN
			RAISE EXCEPTION 'File % not found', full_file_name;
		END IF;
	END LOOP;

	IF (found_file_id IS NULL) THEN
		RAISE EXCEPTION 'File % not found', full_file_name;
	END IF;

	-- If user an admin, he or she can see this file
	IF (is_admin(user_id) = TRUE) THEN
		RETURN found_file_id;
	END IF;

	-- if file was created and was not checked in yet, 
	-- file is exists only for the user who created it, 
	-- as it can be totaly removed with undo operation
	just_created := (SELECT COUNT(*) > 0 FROM File F WHERE F.FileId = found_file_id AND F.CheckedInInstanceId IS NULL);

	IF (just_created = FALSE) THEN
		RETURN found_file_id;
	END IF;
	
	-- Get owner of the file and compare it to user_id
	owner_id := (SELECT UserId FROM CheckOut CO WHERE CO.FileID = found_file_id);

	IF (owner_id <> user_id) THEN
		RAISE EXCEPTION 'File % not found, it was created by another user.', full_file_name;
	END IF;
		
	RETURN found_file_id;
END
$BODY$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION add_or_update_file(
    user_id integer,
    full_parent_file_name text,
    file_name text,
    checkin_comment text,
    file_data bytea)
RETURNS integer AS
$BODY$
DECLARE
    parent_file_id integer;
    file_id integer;
    file_state objectstate;
BEGIN
    -- EXAMPLE: SELECT * FROM add_or_update_file(1, '$root$/MC/', 'ModulesConfigurations.descr', 'Check in cooment', 'file content, can be binary');

    -- get parent file id, exception will occur if it dows not exists
    parent_file_id := get_file_id(user_id, full_parent_file_name);

    -- get file_id id it exists
    BEGIN
        file_id := get_file_id(user_id, parent_file_id, file_name);
    EXCEPTION WHEN OTHERS THEN
        -- File does not exists or it is in state of creation (added by other user but was not checked in yet)
    END;

    IF (file_id IS NULL) THEN
        -- try to add file (it will be checked out)
        file_id	:= (SELECT id FROM add_file(user_id, file_name, parent_file_id, file_data));
    ELSE
		-- check out file if it was not yet
		file_state := get_file_state(file_id);

		IF (file_state.checkedout = FALSE) THEN
			file_state := check_out(user_id, ARRAY[file_id]);

			IF (file_state.checkedout = FALSE) THEN
				RAISE EXCEPTION 'Check out error %', file_name;
			END IF;

		END IF;

		IF (file_state.deleted = TRUE) THEN
			RAISE EXCEPTION 'File %/% marked as deleted, cannot update file.', full_parent_file_name, file_name;
		END IF;

		-- set workcopy
		PERFORM set_workcopy(user_id, file_id, file_data);
    END IF;

    -- check in
    PERFORM check_in(user_id, ARRAY[file_id], checkin_comment);

    RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;

--
-- is_admin (RPCT-93) - fixed: function can return NULL fro non existing user
--
CREATE OR REPLACE FUNCTION is_admin(user_id integer)
RETURNS boolean AS
$BODY$
DECLARE
    result boolean;
BEGIN
	result = (SELECT (Administrator = TRUE AND Disabled = FALSE) AS administrator
		FROM Users WHERE UserID = user_id);

    IF (result is NULL) THEN
        result = FALSE;
    END IF;

    return result;
END;
$BODY$
  LANGUAGE plpgsql;
