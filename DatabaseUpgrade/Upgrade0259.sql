-- RPCT-2251
-- These indexes optimise undo_changes, and relation File<->FileInstance
-- For example delete row from FileInstance leads to serach File.CheckeOutInstanceID, File.CheckedInIntanceID
--
CREATE INDEX file_index_checkedininstanceid ON file USING btree (checkedininstanceid);
CREATE INDEX file_index_checkedoutinstanceid ON file USING btree (checkedininstanceid);

-- check_in, data comparision is changed to compare saved before md5
--
-- Function: public.undo_changes(integer, integer[])
--
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
	checked_out_instance_id uuid;
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
			checked_out_instance_id := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);
			
			UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = file_id;

			-- Delete from file instance all these files
			--
			ALTER TABLE FileInstance DISABLE TRIGGER ALL;
				DELETE FROM FileInstance WHERE FileInstanceID = checked_out_instance_id;
			ALTER TABLE FileInstance ENABLE TRIGGER ALL;

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
