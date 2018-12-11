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
