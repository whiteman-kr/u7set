-- RPCT-1277

-------------------------------------------------------------------------------
--
--		get_specific_copy
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_specific_copy(user_id integer, file_id integer, changeset_id integer)
  RETURNS DbFile AS
$BODY$
DECLARE
	result DbFile;
    actual_changeset_id integer;
		
BEGIN
    actual_changeset_id := (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = file_id AND ChangesetID <= changeset_id);

	SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		CS.ChangesetID AS ChangesetID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data as Data,
		(SELECT count(*) > 0 FROM CheckOut WHERE FileID = file_id) AS CheckedOut,
		--(SELECT Time FROM CheckOut WHERE FileID = file_id) AS CheckOutTime,
		CS.Time As ChechInTime,
		CS.UserID AS UserID,
		FI.Action AS Action,
		FI.Details::text AS Details
	INTO
		result
	FROM
		File F, FileInstance FI, Changeset CS
	WHERE
		F.FileID = file_id AND
		FI.FileID = file_id AND
		FI.ChangesetId = actual_changeset_id AND
		CS.ChangesetID = actual_changeset_id;

	IF (result IS NULL) THEN
		RAISE EXCEPTION 'Cannot find file copy for changeset (FileID: %, ChangesetID: %)', file_id, changeset_id;
	END IF;

	RETURN result;
END;
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		get_specific_copy
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_specific_copy(user_id integer, file_id integer, changeset_data timestamp with time zone)
  RETURNS DbFile AS
$BODY$
DECLARE
	result DbFile;
	actual_changeset_id integer;
		
BEGIN
	actual_changeset_id := (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = file_id AND created <= changeset_data);

	SELECT * INTO result FROM get_specific_copy(user_id, file_id, actual_changeset_id);

	RETURN result;
END;
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--		RPCT-1224
--		check_in
--
-------------------------------------------------------------------------------
DROP FUNCTION check_in(integer, integer[], text);

CREATE OR REPLACE FUNCTION check_in(IN user_id integer, IN file_ids integer[], IN checkin_comment text)
  RETURNS SETOF ObjectState AS
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


-------------------------------------------------------------------------------
--
--							check_in_tree
--
-------------------------------------------------------------------------------
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
					UNION
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
