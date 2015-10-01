-------------------------------------------------------------------------------
--
--							get_file_state
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_file_state(file_id integer)
RETURNS objectstate AS
$BODY$
DECLARE
	checked_out boolean;
	was_deleted boolean;
	action int;
	user_id int;
	return_value ObjectState;
BEGIN
	-- check if file exists
	IF (EXISTS(SELECT * FROM File WHERE FileID = file_id) = FALSE)
	THEN
		RAISE EXCEPTION 'File % is not exists', file_id;
	END IF;

	--
	checked_out := (SELECT count(*) > 0 FROM CheckOut WHERE FileID = file_id);

	IF (checked_out = TRUE)
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

	was_deleted := (SELECT Deleted FROM File WHERE FileID = file_id);

	return_value := ROW(file_id, was_deleted, checked_out, action, user_id, 0);
	RETURN return_value;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							add_file
--
-------------------------------------------------------------------------------

DROP FUNCTION add_file(integer, text, integer, bytea);

CREATE OR REPLACE FUNCTION add_file(user_id INTEGER, file_name TEXT, parent_id INTEGER, file_data bytea, details TEXT)
RETURNS objectstate AS
$BODY$
DECLARE
	exists int;
	newfileid int;
	newfileinstanceid uuid;
	return_value ObjectState;
BEGIN
	SELECT count(*) INTO exists FROM File WHERE Name = file_name AND ParentID = parent_id AND Deleted = false;
	IF (exists > 0) THEN
		RAISE 'Duplicate file name: %', filename USING ERRCODE = 'unique_violation';
	END IF;

	INSERT INTO File (Name, ParentID, Deleted)
		VALUES (file_name, parent_id, false) RETURNING FileID INTO newfileid;

	INSERT INTO CheckOut (UserID, FileID)
		VALUES (user_id, newfileid);

	INSERT INTO FileInstance (FileID, Size, Data, Action, Details)
		VALUES (newfileid, length(file_data), file_data, 1, details::jsonb) RETURNING FileInstanceID INTO newfileinstanceid;

	UPDATE File SET CheckedOutInstanceID = newfileinstanceid WHERE FileID = newfileid;

	return_value := ROW(newfileid, FALSE, TRUE, 1, user_id, 0);
	RETURN return_value;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							add_device
--
-------------------------------------------------------------------------------
DROP FUNCTION add_device(integer, bytea, integer, text);

CREATE OR REPLACE FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text, details TEXT)
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
	SELECT * INTO add_result FROM add_file(user_id, new_filename, parent_id, file_data, details);
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

	if (checkout_user_id <> user_id AND is_admin(user_id) = FALSE)
	THEN
		result := get_file_state(file_id);
		result.errcode := 2;				-- Error, user has no rights
		RETURN result;
	END IF;


	-- check if the file was not checked in yet (at least one time), then remove recors from fileinstance and file, checkout tables;
	checked_out_instance_id := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id AND CheckedInInstanceID IS NULL);

	if (checked_out_instance_id IS NOT NULL)
	THEN
		result := undo_changes(user_id, ARRAY[file_id]);
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
RETURNS SETOF objectstate AS
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
		IF (EXISTS(SELECT * FROM File WHERE FileID = file_id) = FALSE)
		THEN
			file_result := ROW(file_id, true, false, 3, user_id, 0);	-- File was totaly removed
		ELSE
			file_result := get_file_state(file_id);
		END IF;

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

	IF (array_length(checked_out_ids, 1) = 0)
	THEN
		-- Nothing to check in
		RETURN;
	END IF;

	-- Add new record to Changeset
	INSERT INTO Changeset (UserID, Comment, File)
		VALUES (user_id, checkin_comment, TRUE)
		RETURNING ChangesetID INTO NewChangesetID;

	-- Set File.Deleted flag if action in FileInstance is Deleted
	UPDATE File SET Deleted = TRUE
	WHERE
		FileID = ANY(checked_out_ids) AND
		3 = (SELECT Action FROM FileInstance FI WHERE FI.FileInstanceID = CheckedOutInstanceID AND FI.FileID = FileID);

	-- Set CheckedInInstance to current CheckedOutInstance, and set CheckedOutInstanceID to NULL
	UPDATE File SET CheckedInInstanceID = CheckedOutInstanceID WHERE FileID = ANY(checked_out_ids);
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(checked_out_ids);

	-- Update FileInstance, set it's ChangesetID
	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(checked_out_ids) AND ChangesetID IS NULL;

	-- Remove CheckOut roecords
	DELETE FROM CheckOut WHERE FileID = ANY(checked_out_ids);

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
			FileInstance (Data, Size, FileID, ChangesetID, Action, Details)
			SELECT
				Data, length(Data) AS Size, FileId, NULL, 2, Details
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


-------------------------------------------------------------------------------
--
--							dbfile struct
--
-------------------------------------------------------------------------------
CREATE TYPE dbfile AS (
	fileid integer,
	deleted boolean,			-- File or signal was deleted from the table, so such ID is not exists anymore
	name text,
	parentid integer,
	changesetid integer,
	created timestamp with time zone,
	size integer,
	data bytea,
	checkedout boolean,
	checkouttime timestamp with time zone,
	userid integer,
	action integer,
	details text
);


-------------------------------------------------------------------------------
--
--							get_latest_file_version
--
-------------------------------------------------------------------------------
DROP FUNCTION get_latest_file_version(integer, integer);

CREATE OR REPLACE FUNCTION get_latest_file_version(IN user_id integer, IN file_id integer)
  RETURNS SETOF dbfile AS
$BODY$
DECLARE
	is_checked_out boolean;
BEGIN
	SELECT true INTO is_checked_out FROM CheckOut WHERE CheckOut.FileID = file_id;

	IF (is_checked_out = TRUE AND ((SELECT CO.UserID FROM CheckOut CO WHERE CO.FileID = file_id) = user_id OR is_admin(user_id) = TRUE)) THEN
		RETURN QUERY
			SELECT
				F.FileID AS FileID,
				F.Deleted AS Deleted,
				F.Name AS Name,
				F.ParentID AS ParentID,
				0,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				TRUE,	-- Checked_out
				CO.Time As ChechOutOrInTime,	-- CheckOutTime
				CO.UserID AS UserID,
				FI.Action AS Action,
				FI.Details::text AS Details
			FROM
				File F, FileInstance FI, Checkout CO
			WHERE
				F.FileID = file_id AND
				FI.FileInstanceID = F.CheckedOutInstanceID AND
				CO.FileID = file_id AND
				(user_id = CO.UserID OR (SELECT is_admin(user_id)) = TRUE);
	ELSE
		RETURN QUERY
			SELECT
				F.FileID AS FileID,
				F.Deleted AS Deleted,
				F.Name AS Name,
				F.ParentID AS ParentID,
				CS.ChangesetID,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				FALSE,	-- Checked_in
				CS.Time As ChechOutOrInTime,	-- CheckIn time
				CS.UserID AS UserID,
				FI.Action AS Action,
				FI.Details::text AS Details
			FROM
				File F, FileInstance FI, Changeset CS
			WHERE
				F.FileID = file_id AND
				FI.FileInstanceID = F.CheckedInInstanceID AND
				CS.ChangesetID = FI.ChangesetID;
	END IF;
END
$BODY$
  LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--							get_latest_file_tree_version
--
-------------------------------------------------------------------------------

--DROP FUNCTION get_latest_file_tree_version(integer, integer);

CREATE OR REPLACE FUNCTION get_latest_file_tree_version(IN user_id integer, IN file_id integer)
	RETURNS SETOF DbFile AS
$BODY$
DECLARE
	is_checked_out boolean;
	file_ids integer[];
	fid integer;
	file_result DbFile;
BEGIN
	-- Get all files list
	file_ids := array(
					SELECT SQ.FileID FROM (
						(WITH RECURSIVE files(FileID, ParentID) AS (
								SELECT FileID, ParentID FROM get_file_list(user_id, file_id, '%')
							UNION ALL
								SELECT FL.FileID, FL.ParentID FROM Files, get_file_list(user_id, files.FileID, '%') FL
							)
							SELECT * FROM files)
						UNION
							SELECT FileID, ParentID FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE FileID = file_id), '%') WHERE FileID = file_id
					) SQ
					ORDER BY SQ.FileID
				);

	-- Read files latest version
	FOREACH fid IN ARRAY file_ids
	LOOP
		file_result := get_latest_file_version(user_id, fid);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------------------------------------------------------
--
--							get_checked_out_files
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_checked_out_files(user_id integer, parent_file_ids integer[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
	parent_id int;
	checked_out_ids integer[];
BEGIN
	FOREACH parent_id IN ARRAY parent_file_ids
	LOOP
		-- get all checked out files for parents, including parent
		checked_out_ids := array_cat(
			array(
				SELECT SQ.FileID
				FROM (
					(WITH RECURSIVE files(FileID, ParentID, CheckedOut) AS (
							SELECT FileID, ParentID, CheckedOut FROM get_file_list(user_id, parent_id)
						UNION ALL
							SELECT FL.FileID, FL.ParentID, FL.CheckedOut FROM Files, get_file_list(user_id, files.FileID) FL
						)
						SELECT * FROM files)
					UNION
						SELECT FileID, ParentID, CheckedOut FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE FileID = parent_id)) WHERE FileID = parent_id
				) SQ
				WHERE CheckedOut = TRUE
			), checked_out_ids);
	END LOOP;

	IF (array_length(checked_out_ids, 1) = 0)
	THEN
		RETURN;
	END IF;

	-- remove same identifiers, Ids will be sorted
	checked_out_ids := (SELECT * FROM uniq(sort(checked_out_ids)));

	-- Return result
	RETURN QUERY SELECT * FROM get_file_info(user_id, checked_out_ids);
END;
$BODY$
LANGUAGE plpgsql;
