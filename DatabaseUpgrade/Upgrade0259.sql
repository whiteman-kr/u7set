-- RPCT-2251
-- These indexes optimise undo_changes, and relation File<->FileInstance
-- For example delete row from FileInstance leads to serach File.CheckeOutInstanceID, File.CheckedInIntanceID
--
CREATE INDEX file_index_checkedininstanceid ON file USING btree (checkedininstanceid);
CREATE INDEX file_index_checkedoutinstanceid ON file USING btree (checkedoutinstanceid);


CREATE OR REPLACE FUNCTION check_in(
    user_id integer,
    file_ids integer[],
    checkin_comment text)
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	NewChangesetID int;
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
	file_result ObjectState;
	max_file_changeset_id int;
	file_action int;	
	changed_file_ids integer[];
	undo_file_ids integer[];
	last_file_data_md5 text;
	curr_file_data_md5 text;
	was_moved_or_renamed integer;
BEGIN
	-- Check if files really checked out
	is_user_admin :=  is_admin(user_id);

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Check if files really checked out
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			RAISE 'File is not checked out: %', file_id;
		END IF;

		-- Check if the file can be checked_in by this user_id
		file_user := (SELECT UserId FROM CheckOut WHERE FileID = file_id);

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
		END IF;

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
            last_file_data_md5 := (SELECT md5 FROM FileInstance WHERE FileID = file_id AND ChangesetID = max_file_changeset_id);

            -- Get work copy data
            curr_file_data_md5 := (SELECT md5 FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);

			-- If file was moved or renamed we mast check it in
            was_moved_or_renamed := (SELECT COUNT(*) FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL AND (MoveText <> '' OR RenameText <> ''));

            -- Check if data was changed or file was moved or file was renamed
            IF (last_file_data_md5 <> curr_file_data_md5 OR was_moved_or_renamed <> 0)
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
        FOREACH file_id IN ARRAY file_ids
        LOOP
            file_result := get_file_state(file_id);
            RETURN NEXT file_result;
        END LOOP;
        RETURN;        
	END IF;

	-- Add new record to Changeset
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, TRUE) RETURNING ChangesetID INTO NewChangesetID;

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

	-- return table of ObjectState;
	FOREACH file_id IN ARRAY file_ids
	LOOP
		file_result := get_file_state(file_id);
		RETURN NEXT file_result;
	END LOOP;
	RETURN;
END;
$BODY$
LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION undo_changes(
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
	fileinstances_to_remove uuid[];
	checked_out_instance_id uuid;
	checked_in_instance_id uuid;
	files_to_delete int[];
	file_result ObjectState;
	moved_from_parent_id int;
	before_move_file_id int;
	renamed_from text;		-- file was renamed in this check out, then it has initail file name
BEGIN

	is_user_admin := is_admin(user_id);

	-- sort file_ids in descending order, so children will be deleted (if it was not checked in yet) first
	--
	file_ids := uniq(sort_desc(file_ids));

	-- Save fileinstances_to_remove before thei spoiled in the following loop
	--
	fileinstances_to_remove := ARRAY(SELECT CheckedOutInstanceID 
		FROM File 
		WHERE FileId = ANY(file_ids));

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
            SELECT MovedFromParentID, BeforeMoveFileID, RenamedFrom, FileInstanceID
                INTO moved_from_parent_id, before_move_file_id, renamed_from, checked_out_instance_id
                FROM FileInstance 
                WHERE FileID = file_id AND ChangesetID IS NULL;

			SELECT CheckedInInstanceID, CheckedOutInstanceID
                INTO checked_in_instance_id, checked_out_instance_id
                FROM File
                WHERE FileID = file_id;
		
			-- update table file, set CheckedOutIntsnceID to NULL
			--
			UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = file_id;

			-- Delete all check outs
			--
			DELETE FROM CheckOut WHERE FileID = file_id;

			-- if column File.CheckedInIntsnceID is NULL then this file was not checked in, and we have to TRY to remove it along with FileInstance row
			--
			IF (checked_in_instance_id IS NULL) 
			THEN
				files_to_delete := array_append(files_to_delete, file_id);

				file_result.id := file_id;
				file_result.deleted := true;
				file_result.checkedout := false;
				file_result.action := 3;
				file_result.userid := user_id;
				file_result.errcode := 0;				
			ELSE
				file_result := get_file_state(file_id);
			END IF;

			-- It is possible that DELETE FROM File WHERE FileID = file_id; triigers an error if childrean are exists
			--
            EXCEPTION WHEN foreign_key_violation THEN
                -- cannot remove file? mark it as deleted, mark fileinstance action as deleted
                --
                UPDATE FileInstance SET Action = 3 WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

                -- Do not delete it's instance
                --
                fileinstances_to_remove := array_remove(fileinstances_to_remove, checked_out_instance_id);

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

	-- Remove all fileinstances
	--
	DELETE FROM FileInstance WHERE FileInstanceID = ANY(fileinstances_to_remove);
	DELETE FROM File WHERE FileID = ANY(files_to_delete);	

	RETURN;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.check_in_tree(
    session_key text,
	parent_file_ids integer[],
	checkin_comment text)
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
    user_id integer;
	NewChangesetID int;
	file_id int;
	file_result ObjectState;
	checked_out_ids integer[];
	max_file_changeset_id int;
	file_action int;
	changed_file_ids integer[];
	undo_file_ids integer[];
	last_file_data_md5 text;
	curr_file_data_md5 text;
	was_moved_or_renamed integer;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    user_id := user_api.current_user_id(session_key);

    -- get all checked out files for parents (including parent)
	--
	checked_out_ids := api.get_checked_out_files(session_key, parent_file_ids);

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
			last_file_data_md5 := (SELECT md5 FROM FileInstance WHERE FileID = file_id AND ChangesetID = max_file_changeset_id);

            -- Get work copy data
			curr_file_data_md5 := (SELECT md5 FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);

            -- If file was moved or renamed we mast check it in
			was_moved_or_renamed := (SELECT COUNT(*) FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL AND (MoveText <> '' OR RenameText <> ''));

            -- Check if data was changed or file was moved or file was renamed
			IF (last_file_data_md5 <> curr_file_data_md5 OR was_moved_or_renamed <> 0)
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


CREATE OR REPLACE FUNCTION api.get_checked_out_files(
    session_key text,
    parent_file_ids integer[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
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
 		checked_out_ids := array_cat(ARRAY(SELECT FileID FROM api.get_file_list_tree(session_key, parent_id, '%', true) WHERE CheckedOut = TRUE), checked_out_ids);
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
