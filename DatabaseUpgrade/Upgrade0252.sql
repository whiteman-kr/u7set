-- Create columns on table FileInstance 
-- for implementing file renaming
--
ALTER TABLE public.fileinstance
   ADD COLUMN renamedfrom text NOT NULL DEFAULT '';


-- api.rename_file
--
CREATE OR REPLACE FUNCTION api.rename_file(
    session_key text,
    file_id integer,
    new_file_name text)
  RETURNS dbfileinfo AS
$BODY$
DECLARE
	user_id integer;
    file_user_id integer;	
	initial_file_name text;
    already_exists integer;
	parent_id integer;
	checked_in_instance_id uuid;
	checked_out_instance_id uuid;
	rename_text text;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	-- Rename file:
	-- 		1. There is no special action for rename
	--		2. FileInstance has new columns renametext (text for showing in history), renamedfrom (initial file name)
	--		3. If file was just created, then rename operation changes filename and does not fill FilInstance.renametext/renamedfrom
	--		4. FileID is NOT changed
	--
	SELECT Name, ParentId, CheckedInInstanceID, CheckedOutInstanceID
		INTO initial_file_name, parent_id, checked_in_instance_id, checked_out_instance_id
		FROM File 
		WHERE FileId = file_id;
	
	IF (initial_file_name IS NULL) THEN
		RAISE 'Cannot rename file with file id %', file_id;
	END IF;
	
	user_id := user_api.current_user_id(session_key);    

	-- Check if the file is going to be renamed to itself
	--
	IF (initial_file_name = new_file_name) THEN
		-- Then do nothing
		--
		RETURN api.get_file_info(session_key, file_id);
	END IF;

	IF (initial_file_name = new_file_name) THEN
		-- Then do nothing
		--
		RETURN api.get_file_info(session_key, file_id);
	END IF;

	-- Check if the destination file already exists
	--
	already_exists := (SELECT count(*) FROM File WHERE Name = new_file_name AND ParentID = parent_id AND Deleted = false);

	IF (already_exists > 0) THEN
		RAISE 'File % already exists', new_file_name;
	END IF;

	-- Check if the file is checked out and user has right to move file
	-- 
	IF (checked_out_instance_id IS NULL) THEN
		RAISE 'File % is not checked out', file_id;
	END IF;

	file_user_id := (SELECT UserId FROM CheckOut WHERE FileID = file_id);

	IF (user_api.is_current_user_admin(session_key) = FALSE AND file_user_id <> user_id) THEN
		RAISE 'User % has no right to perform operation.', user_id;
	END IF;

	-- Set new file name
	--
	UPDATE File SET Name = new_file_name WHERE FileID = file_id;

	-- If file was just created, then rename it and leave the function, no any changes to FileInstance
	--
	IF (checked_in_instance_id IS NULL) THEN
		RETURN api.get_file_info(session_key, file_id);
	END IF;

	-- Upadte FileInstance
	--

	-- Do not update RenamedFrom if it is not empty, as it has the very first file name, in case of double rename
	--	
	IF ((SELECT RenamedFrom FROM FileInstance WHERE FileInstanceID = checked_out_instance_id) <> '') THEN
		initial_file_name := (SELECT RenamedFrom FROM FileInstance WHERE FileInstanceID = checked_out_instance_id);
	END IF;
	
	rename_text := ('Renamed ' || initial_file_name ||  ' -> ' || new_file_name);

	UPDATE FileInstance 
		SET 
			RenamedFrom = initial_file_name,
			RenameText = rename_text
		WHERE FileInstanceID = checked_out_instance_id;	

	-- That's it
	--
	RETURN api.get_file_info(session_key, file_id);
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION public.undo_changes(
    user_id integer,
    file_ids integer[])
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
	deleted_count int;
	file_result ObjectState;

	moved_from_parent_id int;
	before_move_file_id int;

	renamed_from text;		-- file was renamed in this check out, then it has initail file name
BEGIN

	is_user_admin := is_admin(user_id);

	-- sort file_ids in descending order, so children will be deleted (if it was not checked in yet) first
	--
	file_ids := uniq(sort_desc(file_ids));

	-- undo operation for each file
	--
	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Check if files really checked out
		--
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			-- RAISE 'File is not checked out or file is not exists: %', file_id;
			file_result := get_file_state(file_id);
			RETURN NEXT file_result;
			CONTINUE;
		END IF;

		-- Check if the file can be undo by this user_id
		SELECT UserId INTO file_user FROM CheckOut WHERE FileID = file_id;

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
		END IF;

		-- During removing record from File (if it has not been checked_in before),
		-- if there is any dependants exception can occure
		--
		BEGIN
            -- set moved_from_parent_id, before_move_file_id, renamed_from
            --
            SELECT MovedFromParentID, BeforeMoveFileID, RenamedFrom
                INTO moved_from_parent_id, before_move_file_id, renamed_from
                FROM FileInstance 
                WHERE FileID = file_id AND ChangesetID IS NULL;
		
			-- update table file, set CheckedOutIntsnceID to NULL
			--
			UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = file_id;

			-- Delete from file instance all these files
			--
			DELETE FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL;

			-- Delete all check outs
			--
			DELETE FROM CheckOut WHERE FileID = file_id;

			-- if column File.CheckedInIntsnceID is NULL then this file was not checked in, and we have to TRY to remove it from the table
			--
			DELETE FROM File WHERE FileID = file_id AND (CheckedInInstanceID IS NULL) RETURNING * INTO deleted_count;

			-- form output result
			--
			IF (deleted_count = 0 OR deleted_count IS NULL)
			THEN
				file_result := get_file_state(file_id);
			ELSE
				file_result.id := file_id;
				file_result.deleted := true;
				file_result.checkedout := false;
				file_result.action := 3;
				file_result.userid := user_id;
				file_result.errcode := 0;
			END IF;

            EXCEPTION WHEN foreign_key_violation THEN
                -- cannot remove file? mark it as deleted, mark fileinstance action as deleted
                --
                UPDATE FileInstance SET Action = 3 WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

                -- form output result
                --
                file_result := get_file_state(file_id);
            END;

            -- Restore moved file
            --
           	IF (before_move_file_id <> -1)
			THEN
                -- File was moved, return old file id to tables File and FileInstance
                -- As FileInstance is related to File via FileID, it's impossible just to change FileID in File
                -- (1) So create a new record with old file before_move_file_id, (2) set the old parent to it
                -- and then (3) change FileInstance.Fileid to before_move_file_id
                -- 

                -- Create the new record in Files table, with the new (actually old) file id (before_move_file_id)
                --
                INSERT INTO File (FileID, Name, Created, ParentID, Deleted, CheckedInInstanceId, CheckedOutInstanceID, Attributes) 
                    SELECT before_move_file_id, Name, Created, moved_from_parent_id, Deleted, CheckedInInstanceId, CheckedOutInstanceID, Attributes FROM File WHERE FileID = file_id;

                UPDATE FileInstance SET FileID = before_move_file_id WHERE FileID = file_id;

                -- Delete old record from File (was creted in api.move_file)
                --
                DELETE FROM File WHERE FileID = file_id;
			END IF;

			-- Restore renamed file name
			--
			IF (renamed_from <> '') THEN
				UPDATE File SET Name = renamed_from WHERE FileID = file_id;
			END IF;

		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;