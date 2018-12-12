CREATE OR REPLACE FUNCTION api.get_file_list_tree(
    session_key text,
    parent_id integer,
    file_mask text,
    remove_deleted boolean)
RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    result dbfileinfo[];
BEGIN    
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

	-- Get all files list
	--
	RETURN QUERY 
	(
        WITH RECURSIVE files AS (
            SELECT * FROM api.get_file_list(session_key, parent_id, file_mask) WHERE (Deleted = FALSE) OR (Deleted = TRUE AND remove_deleted = FALSE)
                UNION ALL
            SELECT FL.* 
                FROM Files, api.get_file_list(session_key, files.FileID, file_mask) FL 
                WHERE ((FL.Deleted = FALSE) OR (FL.Deleted = TRUE AND remove_deleted = FALSE)) AND 
                        EXISTS (SELECT * FROM File WHERE File.ParentID = Files.FileID)
        )
	SELECT * FROM files
		UNION ALL
	SELECT * FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = parent_id)) WHERE FileID = parent_id
    );
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_checked_out_files(session_key text, parent_file_ids integer[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    current_user_id integer;
	parent_id int;
	checked_out_ids integer[];
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Get files list
    --
	FOREACH parent_id IN ARRAY parent_file_ids
	LOOP
		-- get all checked out files for parents, including parent
		checked_out_ids := array_cat(
			array(
				SELECT SQ.FileID
				FROM (
					(WITH RECURSIVE files(FileID, ParentID, CheckedOut) AS (
							SELECT FileID, ParentID, CheckedOut FROM api.get_file_list(session_key, parent_id)
						UNION ALL
							SELECT FL.FileID, FL.ParentID, FL.CheckedOut FROM Files, api.get_file_list(session_key, files.FileID) FL
						)
						SELECT * FROM files)
					UNION ALL
						SELECT FileID, ParentID, CheckedOut FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = parent_id)) WHERE FileID = parent_id
				) SQ
				WHERE CheckedOut = TRUE
			), checked_out_ids);
	END LOOP;

	IF (array_length(checked_out_ids, 1) = 0)
	THEN
		RETURN;
	END IF;

	-- Remove same identifiers, Ids will be sorted
	--
	checked_out_ids := (SELECT * FROM uniq(sort(checked_out_ids)));

	-- Return result
	--
	RETURN QUERY SELECT * FROM api.get_file_info(session_key, checked_out_ids);
END;
$BODY$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION api.get_latest_file_tree_version(
    session_key text,
    file_id integer)
  RETURNS SETOF dbfile AS
$BODY$
DECLARE
	is_checked_out boolean;
	file_ids integer[];
	fid integer;
	file_result DbFile;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

	-- Get all files list
	--
	file_ids := array(
					SELECT SQ.FileID FROM (
						(WITH RECURSIVE files(FileID, ParentID) AS (
								SELECT FileID, ParentID FROM api.get_file_list(session_key, file_id) WHERE Deleted = FALSE AND Action <> 3
							UNION ALL
								SELECT FL.FileID, FL.ParentID FROM Files, api.get_file_list(session_key, files.FileID) FL WHERE FL.Deleted = FALSE AND Action <> 3 AND EXISTS (SELECT * FROM File WHERE File.ParentID = Files.FileID)
							)
							SELECT * FROM files)
						UNION ALL
							SELECT FileID, ParentID FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = file_id)) WHERE FileID = file_id
					) SQ
					ORDER BY SQ.FileID
				);

	-- Read files latest version
	--
    RETURN QUERY 
        SELECT * FROM api.get_latest_file_version(session_key, file_ids);

END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION check_in_tree(user_id integer, parent_file_ids integer[], checkin_comment text)
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	NewChangesetID int;
	parent_id int;
	file_id int;
	file_result ObjectState;
	checked_out_ids integer[];
	max_file_changeset_id int;
	file_action int;	
	changed_file_ids integer[];
	undo_file_ids integer[];
	last_file_data bytea;
	curr_file_data bytea;	
BEGIN

	FOREACH parent_id IN ARRAY parent_file_ids
	LOOP
		-- get all checked out files for parents (including parent)
		checked_out_ids := array_cat(
			array(
				SELECT SQ.FileID FROM (
					(WITH RECURSIVE files(FileID, ParentID) AS (
							SELECT FileID, ParentID FROM get_file_list(user_id, parent_id, '%')
						UNION ALL
							SELECT FL.FileID, FL.ParentID FROM Files, get_file_list(user_id, files.FileID, '%') FL
						)
						SELECT * FROM files)
					UNION ALL
						SELECT FileID, ParentID FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE FileID = parent_id), '%') WHERE FileID = parent_id
				) SQ, File AS F
				WHERE SQ.FileID = F.FileID AND F.CheckedOutInstanceID IS NOT NULL
				ORDER BY SQ.FileID
			), checked_out_ids);

	END LOOP;

	IF (array_length(checked_out_ids, 1) = 0 OR 
        array_length(checked_out_ids, 1) IS NULL)
	THEN
		-- Nothing to check in
		RETURN;
	END IF;

	-- remove same ids
	checked_out_ids := uniq(sort_desc(checked_out_ids));

	-- Check if the file was really changed
	FOREACH file_id IN ARRAY checked_out_ids
	LOOP
		-- Check if the file was really changed
        max_file_changeset_id := (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = file_id);
        file_action := (SELECT Action FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);

        IF (max_file_changeset_id IS NULL OR
            file_action <> 2)   -- 2 is Modified
        THEN
            -- File has not been checked in yet
            changed_file_ids := array_append(changed_file_ids,  file_id);
        ELSE
            -- Get last check in data
            last_file_data := (SELECT Data FROM FileInstance WHERE FileID = file_id AND ChangesetID = max_file_changeset_id);

            -- Get work copy data
            curr_file_data := (SELECT Data FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);

            -- Check if data was changed
            IF (last_file_data <> curr_file_data)
            THEN
                changed_file_ids := array_append(changed_file_ids,  file_id);
            ELSE
                undo_file_ids := array_append(undo_file_ids,  file_id);
            END IF;
         END IF;

	END LOOP;

	-- Undo unchanged files
	IF (array_length(undo_file_ids, 1) <> 0) 
	THEN
        PERFORM undo_changes(user_id, undo_file_ids);
	END IF;

	-- Check if changed_file_ids is empty
	IF (array_length(changed_file_ids, 1) = 0 OR 
	    array_length(changed_file_ids, 1) IS NULL)
	THEN
        -- return table of ObjectState;
        FOREACH file_id IN ARRAY checked_out_ids
        LOOP
            file_result := get_file_state(file_id);
            RETURN NEXT file_result;
        END LOOP;
        RETURN;        
	END IF;

	-- Add new record to Changeset
	INSERT INTO Changeset (UserID, Comment, File)
		VALUES (user_id, checkin_comment, TRUE)
		RETURNING ChangesetID INTO NewChangesetID;

	-- Set File.Deleted flag if action in FileInstance is Deleted
	UPDATE File SET Deleted = TRUE
	WHERE
		FileID = ANY(changed_file_ids) AND
		3 = (SELECT Action FROM FileInstance FI WHERE FI.FileInstanceID = CheckedOutInstanceID AND FI.FileID = FileID);

	-- Set CheckedInInstance to current CheckedOutInstance, and set CheckedOutInstanceID to NULL
	UPDATE File SET CheckedInInstanceID = CheckedOutInstanceID WHERE FileID = ANY(changed_file_ids);
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(changed_file_ids);

	-- Update FileInstance, set it's ChangesetID
	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(changed_file_ids) AND ChangesetID IS NULL;

	-- Remove CheckOut roecords
	DELETE FROM CheckOut WHERE FileID = ANY(changed_file_ids);

	-- Return result
	FOREACH file_id IN ARRAY checked_out_ids
	LOOP
		file_result := get_file_state(file_id);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END;
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION get_file_history_recursive(user_id integer, parent_id integer)
	RETURNS SETOF DbChangeset AS
$BODY$
DECLARE
	tree_files_ids integer[];
BEGIN
	tree_files_ids := 
		array(
			SELECT SQ.FileID FROM (
				(WITH RECURSIVE files(FileID, ParentID) AS (
						SELECT FL1.FileID, FL1.ParentID FROM get_file_list(user_id, parent_id) AS FL1
					UNION ALL
						SELECT FL2.FileID, FL2.ParentID FROM Files, get_file_list(user_id, files.FileID) FL2
					)
					SELECT * FROM files)
				UNION ALL
					SELECT FL3.FileID, FL3.ParentID FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE File.FileID = parent_id)) AS FL3 WHERE FL3.FileID = parent_id
			) SQ
		);

	RETURN QUERY
		SELECT DISTINCT
			Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Users.Username AS Username,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			0 AS Action
		FROM
			FileInstance, Changeset, Users
		WHERE
			FileInstance.FileID = ANY(tree_files_ids) AND
			FileInstance.ChangesetID =  Changeset.ChangesetID AND
			Changeset.UserID = Users.UserID
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
LANGUAGE plpgsql;


-- Create $root$/Schemas
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$', 'Schemas', 'Update: Adding file $root$/Schemas', '', '{}');

-- Rename Schemas
--	AL->ApplicationLogic
--      MVS->Monitor
--	WVS->Wiring
--	DVS->Diagnostics
--
UPDATE File SET Name = 'ApplicationLogic' WHERE Name = 'AL' AND ParentID = 0;
UPDATE File SET Name = 'Monitor' WHERE Name = 'MVS' AND ParentID = 0;
UPDATE File SET Name = 'Wiring' WHERE Name = 'WVS' AND ParentID = 0;
UPDATE File SET Name = 'Diagnostics' WHERE Name = 'DVS' AND ParentID = 0;

-- Set parent for schemas, $root$/ApplicationLogic -> $root$/Schemas/ApplicationLogic
--
UPDATE File
    SET ParentID = (SELECT FileID FROM File WHERE Name = 'Schemas' AND ParentID = 0)
    WHERE   (Name = 'ApplicationLogic' OR 
            Name = 'Monitor' OR
            Name = 'Wiring' OR 
            Name = 'Diagnostics' OR
            Name = 'UFBL') AND ParentID = 0;

-- Create $root$/Schemas/Tuning
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$/Schemas', 'Tuning', 'Update: Adding file $root$/Schemas', '', '{}');



-- Create user_api version of get_file_id
--
CREATE OR REPLACE FUNCTION api.get_file_id(
    session_key text,
    full_file_name text)
  RETURNS integer AS
$BODY$
DECLARE
    user_id integer;
	split_names text[];
	fn text;
	found_file_id integer;
	just_created boolean;
	owner_id int;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);

	-- full_file_name -- file with full path, example '$root$/MC/ModuleConfigration.jscript'
	-- EXAMPLE: SELECT * FROM api.get_file_id('87408329edhewjhwef890', '///$root$/HC/device-cc6cd30a-defb-11e4-bdb2-5238966c092f.hsm///');

	-- Find file
 	--
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
	--
	IF (user_api.is_current_user_admin(session_key) = TRUE) THEN
		RETURN found_file_id;
	END IF;

	-- if file was created and was not checked in yet, 
	-- file exists only for the user who created it, 
	-- as it can be totaly removed with undo operation
	--
	just_created := (SELECT COUNT(*) > 0 FROM File F WHERE F.FileId = found_file_id AND F.CheckedInInstanceId IS NULL);

	IF (just_created = FALSE) THEN
		RETURN found_file_id;
	END IF;
	
	-- Get owner of the file and compare it to user_id
	--
	owner_id := (SELECT UserId FROM CheckOut CO WHERE CO.FileID = found_file_id);

	IF (owner_id <> user_id) THEN
		RAISE EXCEPTION 'File % not found, it was created by another user.', full_file_name;
	END IF;
		
	RETURN found_file_id;
END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_file_id(
    session_key text,
    parent_id integer,
    file_name text)
  RETURNS integer AS
$BODY$
DECLARE
    user_id integer;
	found_file_id integer;
	just_created boolean;
	owner_id int;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    user_id := user_api.current_user_id(session_key);

	-- find file
	--
	found_file_id := (SELECT F.FileID 
				FROM File F 
				WHERE F.ParentId = parent_id AND 
					F.Name = file_name AND
					F.Deleted = FALSE);

	IF (found_file_id IS NULL) THEN
		RAISE EXCEPTION 'File % not found', file_name;
	END IF;

	-- If user an admin, he or she can see this file
	--
	IF (user_api.is_current_user_admin(session_key) = TRUE) THEN
		RETURN found_file_id;
	END IF;

	-- if file was created and was not checked in yet, 
	-- file exists only for the user who created it, 
	-- as it can be totaly removed by undo operation
	--
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


-- Function: api.add_or_update_file(text, text, text, text, bytea, text)

-- DROP FUNCTION api.add_or_update_file(text, text, text, text, bytea, text);

CREATE OR REPLACE FUNCTION api.add_or_update_file(
    session_key text,
    full_parent_file_name text,
    file_name text,
    checkin_comment text,
    file_data bytea,
    details text)
  RETURNS integer AS
$BODY$
DECLARE
	current_user_id integer;
    parent_file_id integer;
    file_id integer;
    file_state objectstate;
BEGIN
	-- EXAMPLE: SELECT * FROM add_or_update_file($(S_e_s_s_i_o_n_K_e_y), '$root$/MC/', 'ModulesConfigurations.descr', 'Check in cooment', 'file content, can be binary', '{}');
	--

    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	current_user_id := user_api.current_user_id(session_key);

    -- get parent file id, exception will occur if it dows not exists
    --
    parent_file_id := api.get_file_id(session_key, full_parent_file_name);

    -- get file_id if exists
    --
    BEGIN
        file_id := api.get_file_id(session_key, parent_file_id, file_name);
    EXCEPTION WHEN OTHERS THEN
        -- File does not exists or it is in state of creation (added by other user but was not checked in yet)
    END;

    IF (file_id IS NULL) THEN
        -- try to add file (it will be checked out)
		file_id	:= (SELECT id FROM add_file(current_user_id, file_name, parent_file_id, file_data, details));
    ELSE
		-- check out file if it was not yet
		file_state := get_file_state(file_id);

		IF (file_state.checkedout = FALSE) THEN
			file_state := check_out(current_user_id, ARRAY[file_id]);

			IF (file_state.checkedout = FALSE) THEN
				RAISE EXCEPTION 'Check out error %', file_name;
			END IF;

		END IF;

		IF (file_state.deleted = TRUE) THEN
			RAISE EXCEPTION 'File %/% marked as deleted, cannot update file.', full_parent_file_name, file_name;
		END IF;

		-- set workcopy
		PERFORM api.set_workcopy(session_key, file_id, file_data, details);
    END IF;

    -- check in
    PERFORM check_in(current_user_id, ARRAY[file_id], checkin_comment);

    RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION api.get_file_info(
	session_key text,
	full_path_file_name text)
RETURNS dbfileinfo AS
$BODY$
DECLARE
	file_id integer;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	file_id := api.get_file_id(session_key, full_path_file_name);

	RETURN api.get_file_info(session_key, file_id);
END
$BODY$
LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION api.get_file_info(
	session_key text,
	full_path_file_name text[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
	file_id integer;
	file_info dbfileinfo;
	fn text;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	FOREACH fn IN ARRAY full_path_file_name
	LOOP
		file_id := api.get_file_id(session_key, fn);
		file_info := api.get_file_info(session_key, file_id);
		
		RETURN NEXT file_info;
	END LOOP;

	RETURN;
END
$BODY$
  LANGUAGE plpgsql;
