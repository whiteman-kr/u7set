-- RPCT-2251
-- These indexes optimise undo_changes, and relation File<->FileInstance
-- For example delete row from FileInstance leads to serach File.CheckeOutInstanceID, File.CheckedInIntanceID
--
CREATE INDEX file_index_checkedininstanceid ON file USING btree (checkedininstanceid);
CREATE INDEX file_index_checkedoutinstanceid ON file USING btree (checkedininstanceid);

-- check_in, data comparision is changed to compare saved before md5
--
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
