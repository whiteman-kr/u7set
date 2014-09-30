-------------------------------------------------------------------------------
--
--							get_file_state
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_file_state(file_id integer)
RETURNS ObjectState AS
$BODY$
DECLARE
	checked_out boolean;
	action int;
	user_id int;
BEGIN
	checked_out := (SELECT count(*) > 0 FROM CheckOut WHERE FileID = file_id);

	if (checked_out = TRUE)
	THEN
		action := (SELECT FI.action
				FROM File F, FileInstance FI
				WHERE F.FileID = file_id AND FI.FileInstanceID = F.CheckedOutInstanceID);

		user_id := (SELECT UserID FROM CheckOut WHERE FileID = file_id);
	ELSE
		action := (SELECT FI.action
				FROM File F, FileInstance FI
				WHERE F.FileID = file_id AND FI.FileInstanceID = F.CheckedInInstanceID);

		user_id := (SELECT UserID
				FROM
					File F, FileInstance FI, Changeset CS
				WHERE
					F.FileID = file_id AND
					FI.FileInstanceID = F.CheckedInInstanceID AND
					CS.ChangesetID = FI.ChangesetID);
	END IF;


	RETURN ROW(file_id, (SELECT count(*) = 0 FROM File WHERE FileID = file_id), checked_out, action, user_id, 0);
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							add_file
--
-------------------------------------------------------------------------------

DROP FUNCTION add_file(integer, text, integer, bytea);

CREATE OR REPLACE FUNCTION add_file(user_id integer, file_name text, parent_id integer, file_data bytea)
RETURNS ObjectState AS
$BODY$
DECLARE
	exists int;
	newfileid int;
	newfileinstanceid uuid;
BEGIN
	SELECT count(*) INTO exists FROM File WHERE Name = file_name AND ParentID = parent_id AND Deleted = false;
	IF (exists > 0) THEN
		RAISE 'Duplicate file name: %', filename USING ERRCODE = 'unique_violation';
	END IF;

	INSERT INTO File (Name, ParentID, Deleted)
		VALUES (file_name, parent_id, false) RETURNING FileID INTO newfileid;

	INSERT INTO CheckOut (UserID, FileID)
		VALUES (user_id, newfileid);

	INSERT INTO FileInstance (FileID, Size, Data, Action)
		VALUES (newfileid, length(file_data), file_data, 1) RETURNING FileInstanceID INTO newfileinstanceid;

	UPDATE File SET CheckedOutInstanceID = newfileinstanceid WHERE FileID = newfileid;

	RETURN ROW(newfileid, FALSE, TRUE, 1, user_id, 0);
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							add_device
--
-------------------------------------------------------------------------------
DROP FUNCTION add_device(integer, bytea, integer, text);

CREATE OR REPLACE FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text)
RETURNS ObjectState AS
$BODY$
DECLARE
	fileid_lenght int;
	new_filename text;
	add_result ObjectState;
BEGIN
	-- TO DO: Check user right here

	-- generate filename
	SELECT * INTO new_filename FROM uuid_generate_v1();
	SELECT octet_length(new_filename) INTO fileid_lenght;

	new_filename := 'device-' || new_filename || file_extension;	-- smthng like: device-5be363ac-3c02-11e4-9de8-3f84f459cb27.hsystem

	-- add new file
	SELECT * INTO add_result FROM add_file(user_id, new_filename, parent_id, file_data);
	RETURN add_result;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							delete_file
--
-------------------------------------------------------------------------------
DROP FUNCTION delete_file(integer, integer);

CREATE OR REPLACE FUNCTION delete_file(user_id integer, file_id integer)
  RETURNS ObjectState AS
$BODY$
DECLARE
	check_out_result ObjectState;
	checkout_user_id int;
	checked_out_instance_id uuid;
	result ObjectState;
BEGIN
	-- CheckOut file if it has not been yet
	IF is_file_checkedout(file_id) = FALSE
	THEN
		check_out_result := check_out(user_id, ARRAY[file_id]);

		IF (check_out_result.CheckedOut <> TRUE OR check_out_result.UserID <> user_id)
		THEN
			result := get_file_state(file_id);
			result.errcode := 1;			-- Error, can't check out file
			RETURN result;
		END IF;
	END IF;

	-- get check out user id, file cout be checked out by other user, we must check it
	checkout_user_id := (SELECT UserID FROM CheckOut WHERE FileID = file_id);

	if (checkout_user_id <> user_id OR is_admin(user_id) = FALSE)
	THEN
		result := get_file_state(file_id);
		result.errcode := 2;				-- Error, user has no rights
		RETURN result;
	END IF;


	-- check if the file was not checked in yet (at least one time), then remove recors from fileinstance and file, checkout tables;
	checked_out_instance_id := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id AND CheckedInInstanceID IS NULL);

	if (checked_out_instance_id IS NOT NULL)
	THEN
		PERFORM undo_changes(user_id, ARRAY[file_id]);

		result := get_file_state(file_id);
		RETURN result;
	END IF;

	-- mark fileinstance action as deleted
	UPDATE FileInstance SET Action = 3 WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

	result := get_file_state(file_id);
	RETURN result;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							undo_changes
--
-------------------------------------------------------------------------------
DROP FUNCTION undo_changes(integer, integer[]);

CREATE OR REPLACE FUNCTION undo_changes(user_id integer, file_ids integer[])
RETURNS SETOF ObjectState AS
$BODY$
DECLARE
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
	file_result ObjectState;
BEGIN

	is_user_admin := is_admin(user_id);

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Check if files really checked out
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			RAISE 'File is not checked out: %', file_id;
		END IF;

		-- Check if the file can be undo by this user_id
		SELECT UserId INTO file_user FROM CheckOut WHERE FileID = file_id;

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
		END IF;
	END LOOP;

	-- update table file, set CheckedOutIntsnceID to NULL
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(file_ids);

	-- Delete from file instance all these files
	DELETE FROM FileInstance WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	-- Delete all check outs
	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);


	-- if column File.CheckedInIntsnceID is NULL then this file was not checked in, and we have to remove it from the table
	DELETE FROM File WHERE FileID = ANY(file_ids) AND CheckedInInstanceID IS NULL;

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
--							check_in
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
BEGIN
	-- Check if files really checked out
	SELECT is_admin(user_id) INTO is_user_admin;

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Check if files really checked out
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			RAISE 'File is not checked out: %', file_id;
		END IF;

		-- Check if the file can be undo by this user_id
		SELECT UserId INTO file_user FROM CheckOut WHERE FileID = file_id;

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
		END IF;
	END LOOP;

	-- Add new record to Changeset
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, TRUE) RETURNING ChangesetID INTO NewChangesetID;

	-- Set File.Deleted flag if action in FileInstance is Deleted
	UPDATE File SET Deleted = TRUE
		WHERE
			FileID = ANY(file_ids) AND
			3 = (SELECT Action FROM FileInstance FI WHERE FI.FileInstanceID=CheckedOutInstanceID AND FI.FileID=FileID);

	-- Set CheckedInInstance to current CheckedOutInstance, and set CheckedOutInstanceID to NULL
	UPDATE File SET CheckedInInstanceID = CheckedOutInstanceID WHERE FileID = ANY(file_ids);
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(file_ids);

	-- Update FileInstance, set it's ChangesetID
	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	-- Remove CheckOut roecords
	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);

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
--							check_out
--
-------------------------------------------------------------------------------
DROP FUNCTION check_out(integer, integer[]);

CREATE OR REPLACE FUNCTION check_out(IN user_id integer, IN file_ids integer[])
RETURNS SETOF ObjectState AS
$BODY$
DECLARE
	AlreadyCheckedOut integer;
	file_id integer;
	checkout_instance_id uuid;
	file_result ObjectState;
BEGIN
	-- Check if file is not checked out already
	SELECT count(CheckOutID) INTO AlreadyCheckedOut FROM CheckOut WHERE FileID = ANY(file_ids);
	IF (AlreadyCheckedOut > 0) THEN
		RAISE 'Files already checked out';
	END IF;

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Add record to the CheckOutTable
		INSERT INTO CheckOut (UserID, FileID) VALUES (user_id, file_id);

		-- Make new work copy in FileInstance
		INSERT INTO
			FileInstance (Data, Size, FileID, ChangesetID, Action)
			SELECT
				Data, length(Data) AS Size, FileId, NULL, 2
			FROM
				FileInstance
			WHERE
				FileID = file_id AND
				FileInstanceID = (SELECT CheckedInInstanceID FROM File WHERE FileID = file_id)
			RETURNING FileInstanceID INTO checkout_instance_id;

		-- Set CheckedOutInstanceID for File table
		UPDATE File SET CheckedOutInstanceID = checkout_instance_id WHERE FileID = file_id;

	END LOOP;

	-- Return just checked out files
	FOREACH file_id IN ARRAY file_ids
	LOOP
		file_result := get_file_state(file_id);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;
