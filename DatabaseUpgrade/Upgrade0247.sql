-- Create columns on table FileInstance 
-- for implementing file moving
--
ALTER TABLE public.fileinstance
   ADD COLUMN beforemovefileid integer NOT NULL DEFAULT -1;

ALTER TABLE public.fileinstance
   ADD COLUMN movedfromparentid integer NOT NULL DEFAULT -1;

ALTER TABLE public.fileinstance
   ADD COLUMN movedtoparentid integer NOT NULL DEFAULT -1;

ALTER TABLE public.fileinstance
   ADD COLUMN movetext text NOT NULL DEFAULT '';

-- Get full function name with path by file id
-- returns like $root$/MC/File.txt
--
CREATE OR REPLACE FUNCTION api.get_file_full_path(session_key text, file_id integer)
    RETURNS Text AS
$BODY$
DECLARE
    files_array text[];
    result text;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	files_array := ARRAY
	        (
            WITH RECURSIVE files_to_parent AS 
                (
                    SELECT FileID, Name, ParentID, 1 AS Depth FROM File WHERE FileID = file_id
                        UNION ALL
                    SELECT F.FileID, F.Name, F.ParentID, ftp.Depth + 1 FROM files_to_parent ftp, File F WHERE F.FileID = FTP.ParentID
                )
                SELECT Name FROM files_to_parent ORDER BY Depth DESC
        );

    result := array_to_string(files_array, '/');
    return result;
END
$BODY$
LANGUAGE plpgsql;

-- Move file
--
CREATE OR REPLACE FUNCTION api.move_file(
    session_key text,
    file_id integer,
    move_to_parent_id integer)
    RETURNS objectstate AS
$BODY$
DECLARE
    already_exists int;
    file_has_children int;
    file_name text;
    user_id integer;
    file_user_id integer;
	new_file_id int;
	move_from_parent_id int;
	fileinstance_uuid uuid;
	move_text text;
	return_value ObjectState;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Move file:
    --      1. There is no special action for move
    --      2. FileInstance has three new columns MovedFromParentId, BeforeMoveFileId, MoveText
    --      3. Moved file or not is determined by FileInstance.MovedFromParentId
    --      4. If file was just created, then move operation just moves it and DOES NOT fill FileInstance, so it will count as created in the new location
    --      5. Destination file has new FileID -- it is important that children always has file id bigger then their parent (the same for 4.)
    --

	file_name := (SELECT Name FROM File WHERE FileId = file_id);
	user_id := user_api.current_user_id(session_key);    
	
    -- Check if the file is going to be moved to itself
    --
    move_from_parent_id := (SELECT ParentId FROM File WHERE FileId = file_id);
    
	IF (move_from_parent_id = move_to_parent_id) THEN
	    RAISE 'File % cannot be moved to the same parent', file_name;
	END IF;

	-- File with children cannot be moved
	--
	file_has_children := (SELECT COUNT(*) FROM File WHERE ParentID = file_id AND Deleted = FALSE);

	IF (file_has_children > 0) THEN
	    RAISE 'File % cnnot be moved as it has % child(ren)', file_name, file_has_children;
	END IF;	

    -- Check if the destination file already exists
    --
    already_exists := (SELECT count(*) FROM File WHERE Name = file_name AND ParentID = move_to_parent_id AND Deleted = false);

	IF (already_exists > 0) THEN
	    RAISE 'File % already exists', file_name;
	END IF;

	-- Check if the file is checked out and user has right to move file
	-- 
	fileinstance_uuid := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

    IF (fileinstance_uuid IS NULL) THEN
	    RAISE 'File % is not checked out', file_id;
	END IF;

	file_user_id := (SELECT UserId FROM CheckOut WHERE FileID = file_id);

	IF (user_api.is_current_user_admin(session_key) = FALSE AND file_user_id <> user_id) THEN
		RAISE 'User % has no right to perform operation.', user_id;
	END IF;

    -- Create the new record in Files table, with the new FileID
    --
    INSERT INTO File (Name, Created, ParentID, Deleted, CheckedInInstanceId, CheckedOutInstanceID, Attributes) 
        SELECT Name, Created, ParentID, Deleted, CheckedInInstanceId, CheckedOutInstanceID, Attributes FROM File WHERE FileID = file_id
        RETURNING FileID INTO new_file_id;

    -- Set new parent to just created File record
    --
    UPDATE File SET ParentId = move_to_parent_id WHERE FileID = new_file_id;    

    -- move_text
    --
    move_text := 'Moved from ' || api.get_file_full_path(session_key, move_from_parent_id) ||  ' to ' || api.get_file_full_path(session_key, move_to_parent_id);

    -- Update tables CheckOut (FileID), FileInstances (FileID, MovedFromParentID, BeforeMoveParentID, MoveText)
    --
    UPDATE CheckOut SET FileID = new_file_id WHERE FileID = file_id;    

    UPDATE FileInstance SET FileID = new_file_id WHERE FileID = file_id;

    UPDATE FileInstance 
        SET 
            MovedFromParentID = move_from_parent_id,
            MovedToParentID = move_to_parent_id,
            BeforeMoveFileID = file_id,
            MoveText = move_text
        WHERE FileInstanceID = fileinstance_uuid;

    -- Delete old record from File
    --
    DELETE FROM File WHERE FileID = file_id;

    -- That's it
    --
    SELECT F.FileID, F.Deleted, true, FI.Action, CO.UserID, 0 
        INTO return_value
        FROM File F, FileInstance FI, CheckOut CO 
        WHERE F.FileId = new_file_id AND FI.FileInstanceID = F.CheckedOutInstanceID AND CO.FileID = new_file_id;
    
 	RETURN return_value;
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
            -- set moved_from_parent_id, before_move_file_id
            --
            SELECT MovedFromParentID, BeforeMoveFileID 
                INTO moved_from_parent_id, before_move_file_id
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

		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;
