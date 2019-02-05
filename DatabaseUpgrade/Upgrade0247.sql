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

ALTER TABLE public.fileinstance
   ADD COLUMN renametext text NOT NULL DEFAULT '';

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
  RETURNS dbfileinfo AS
$BODY$
DECLARE
    already_exists int;
    file_has_children int;
    file_name text;
    user_id integer;
    file_user_id integer;
    initial_file_id int;            -- This file id when file was checked out or created, as two move opeartion can take place during singles checkout we don't want to spoil initial file id ( If FileInstance.BeforeMoveFileID != -1) then initial_file_id = FileInstance.BeforeMoveFileID;
	new_file_id int;
	move_from_parent_id int;
	fileinstance_uuid uuid;
	move_text text;
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
	    RAISE 'File % cannot be moved as it has % child(ren)', file_name, file_has_children;
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

	-- This file id when file was checked out or created, as two move opeartion can take place during singles checkout we don't want to spoil initial file id 
	-- If FileInstance.BeforeMoveFileID != -1) then initial_file_id = FileInstance.BeforeMoveFileID;
	--
	initial_file_id := (SELECT BeforeMoveFileID FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);
	IF (initial_file_id = -1) THEN
        initial_file_id := file_id;
	END IF;

	IF ((SELECT MovedFromParentID FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL) <> -1) THEN
        move_from_parent_id := (SELECT MovedFromParentID FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);  -- If two move oparations were, then take initaial move_from_parent_id
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
            BeforeMoveFileID = initial_file_id, 
            MoveText = move_text
        WHERE FileInstanceID = fileinstance_uuid;

    -- Delete old record from File
    --
    DELETE FROM File WHERE FileID = file_id;

    -- That's it
    --
    --SELECT F.FileID, F.Deleted, true, FI.Action, CO.UserID, 0 
    --    INTO return_value
    --    FROM File F, FileInstance FI, CheckOut CO 
    --    WHERE F.FileId = new_file_id AND FI.FileInstanceID = F.CheckedOutInstanceID AND CO.FileID = new_file_id;

    RETURN api.get_file_info(session_key, new_file_id);
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.move_files(
	session_key text,
	file_ids integer[],
	move_to_parent_id integer)
	RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
	file_id integer;
	file_result dbfileinfo;
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	-- Move files
	--
	FOREACH file_id IN ARRAY file_ids
	LOOP
		file_result := api.move_file(session_key, file_id, move_to_parent_id);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
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


-- if file was moved this field has move text like $root/Schema moved to $root$/MC
--
ALTER TYPE public.dbchangesetdetails
	ADD ATTRIBUTE filemovetext text;	

-- if file was renamed it has text like aaa.txt->bbb.txt
--
ALTER TYPE public.dbchangesetdetails
	ADD ATTRIBUTE filerenametext text;  


CREATE OR REPLACE FUNCTION get_changeset_details(user_id integer, changeset_id integer )
	RETURNS SETOF DbChangesetDetails AS
$BODY$
BEGIN
    -- Check changeset
    --
    IF (SELECT COUNT(*) FROM Changeset WHERE ChangesetID = changeset_id) = 0
    THEN
        RAISE 'Changeset % not found.', changeset_id;
    END IF;

    RETURN QUERY
		SELECT 
                -- DbChangeset
                CS.ChangesetID AS ChangesetID,
                CS.UserID AS UserID,
                U.Username AS Username,
                CS.Time AS CheckInTime,
                CS.Comment AS Comment,
                FI.Action AS Action,                -- Action defined in DbStruct.VcsItemAction
                -- DbChangesetObject
                0 AS ObjectType,                    -- 0 - file, 1 - signal
				F.FileID AS ObjectID,
                F.Name AS ObjectName, 
                F.Name AS ObjectCaption,
                FI.Action AS ObjectAction,          -- Action defined in DbStruct.VcsItemAction
                F.ParentID::text AS ObjectParent,
                FI.MoveText AS filemovetext,
                FI.RenameText AS filerenametext				-- not implemented yet
            FROM 
                Changeset CS,
                FileInstance FI,
                File F,
                Users U
            WHERE
                CS.ChangesetID = changeset_id AND
                FI.ChangesetID = CS.ChangesetID AND
                FI.FileID = F.FileID AND
                CS.UserID = U.UserID
        UNION ALL
		SELECT 
                -- DbChangeset
                CS.ChangesetID AS ChangesetID,
                CS.UserID AS UserID,
                U.Username AS Username,
                CS.Time AS CheckInTime,
                CS.Comment AS Comment,  
                SI.Action AS Action,                -- Action defined in DbStruct.VcsItemAction
                -- DbChangesetObject
                1 AS ObjectType,                    -- 0 - file, 1 - signal
				S.SignalID AS ObjectID,
                SI.AppSignalID AS ObjectName, 
                SI.Caption AS ObjectCaption,
                SI.Action AS ObjectAction,          -- Action defined in DbStruct.VcsItemAction
                SI.EquipmentID AS ObjectParent,
				'' AS filemovetext,					-- signal does not have this feature
                '' AS filerenametext				-- signal does not have this feature
            FROM 
                Changeset CS,
                SignalInstance SI,
                Signal S,
                Users U
            WHERE
                CS.ChangesetID = changeset_id AND
                SI.ChangesetID = CS.ChangesetID AND
                SI.SignalID = S.SignalID AND
                CS.UserID = U.UserID;            
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION public.check_in(
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
	last_file_data bytea;
	curr_file_data bytea;
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
            last_file_data := (SELECT Data FROM FileInstance WHERE FileID = file_id AND ChangesetID = max_file_changeset_id);

            -- Get work copy data
            curr_file_data := (SELECT Data FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL);

			-- If file was moved or renamed we mast check it in
            was_moved_or_renamed := (SELECT COUNT(*) FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL AND (MoveText <> '' OR RenameText <> ''));

            -- Check if data was changed or file was moved or file was renamed
            IF (last_file_data <> curr_file_data OR was_moved_or_renamed <> 0)
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


-- Function: public.check_in_tree(integer, integer[], text)
--
CREATE OR REPLACE FUNCTION public.check_in_tree(
    user_id integer,
    parent_file_ids integer[],
    checkin_comment text)
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
	was_moved_or_renamed integer;
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

			-- If file was moved or renamed we mast check it in
            was_moved_or_renamed := (SELECT COUNT(*) FROM FileInstance WHERE FileID = file_id AND ChangesetID IS NULL AND (MoveText <> '' OR RenameText <> ''));            

            -- Check if data was changed or file was moved or file was renamed
            IF (last_file_data <> curr_file_data OR was_moved_or_renamed <> 0)
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
