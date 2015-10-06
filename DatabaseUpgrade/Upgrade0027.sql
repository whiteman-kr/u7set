-------------------------------------------------------------------------------
--
--							dbfileinfo struct
--
-------------------------------------------------------------------------------
CREATE TYPE dbfileinfo AS
   (fileid integer,
	deleted boolean,
	name text,
	parentid integer,
	changesetid integer,
	created timestamp with time zone,
	size integer,
	checkedout boolean,
	checkouttime timestamp with time zone,
	userid integer,
	action integer,
	details text);

-------------------------------------------------------------------------------
--
--							get_file_list
--
-------------------------------------------------------------------------------
DROP FUNCTION get_file_list(integer, integer, text);

CREATE OR REPLACE FUNCTION get_file_list(IN user_id integer, IN parent_id integer, IN file_mask text)
	RETURNS	SETOF DbFileInfo AS
$BODY$

(SELECT
	F.FileID AS FileID,
	F.Deleted AS Deleted,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.ChangesetID AS ChangesetID,
	F.Created AS Created,
	F.Size AS Size,
	F.ChangesetID IS NULL AS CheckedOut,
	Changeset.time AS CheckOutTime,
	Changeset.UserID AS UserID,
	F.Action AS Action,
	F.Details AS Details
FROM
	-- All checked in now
	(SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action,
		FI.Details::text AS Details
	FROM
		File F,
		FileInstance FI
	WHERE
		F.ParentID = parent_id AND
		F.CheckedInInstanceID = FI.FileInstanceID AND
		F.CheckedOutInstanceID IS NULL AND
		F.FileID = FI.FileID AND
		F.Name ILIKE file_mask
	) AS F
	LEFT JOIN
	Changeset USING (ChangesetID))
UNION
(SELECT
	F.FileID AS FileID,
	F.Deleted AS Deleted,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.ChangesetID AS ChangesetID,
	F.Created AS Created,
	F.Size AS Size,
	F.ChangesetID IS NULL AS CheckedOut,
	CheckOut.time AS CheckOutTime,
	CheckOut.UserID AS UserID,
	F.Action AS Action,
	F.Details AS Details
FROM
	-- All CheckedOut by any user if user_id is administrator
	(SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action,
		FI.Details::text AS Details
	FROM
		File F,
		FileInstance FI,
		CheckOut CO
	WHERE
		F.ParentID = parent_id AND
		F.CheckedOutInstanceID = FI.FileInstanceID AND
		F.FileID = FI.FileID AND
		F.FileID = CO.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE) AND
		F.Name ILIKE file_mask
	) AS F
	LEFT JOIN
	CheckOut USING (FileID))
ORDER BY Name;

$BODY$
LANGUAGE sql;


-------------------------------------------------------------------------------
--
--							get_file_list	without checking file name
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_file_list(user_id integer, parent_id integer)
  RETURNS SETOF dbfileinfo AS
$BODY$

(SELECT
	F.FileID AS FileID,
	F.Deleted AS Deleted,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.ChangesetID AS ChangesetID,
	F.Created AS Created,
	F.Size AS Size,
	F.ChangesetID IS NULL AS CheckedOut,
	Changeset.time AS CheckOutTime,
	Changeset.UserID AS UserID,
	F.Action AS Action,
	F.Details AS Details
FROM
	-- All checked in now
	(SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action,
		FI.Details::text AS Details
	FROM
		File F,
		FileInstance FI
	WHERE
		F.ParentID = parent_id AND
		F.CheckedInInstanceID = FI.FileInstanceID AND
		F.CheckedOutInstanceID IS NULL AND
		F.FileID = FI.FileID
	) AS F
	LEFT JOIN
	Changeset USING (ChangesetID))
UNION
(SELECT
	F.FileID AS FileID,
	F.Deleted AS Deleted,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.ChangesetID AS ChangesetID,
	F.Created AS Created,
	F.Size AS Size,
	F.ChangesetID IS NULL AS CheckedOut,
	CheckOut.time AS CheckOutTime,
	CheckOut.UserID AS UserID,
	F.Action AS Action,
	F.Details AS Details
FROM
	-- All CheckedOut by any user if user_id is administrator
	(SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action,
		FI.Details::text AS Details
	FROM
		File F,
		FileInstance FI,
		CheckOut CO
	WHERE
		F.ParentID = parent_id AND
		F.CheckedOutInstanceID = FI.FileInstanceID AND
		F.FileID = FI.FileID AND
		F.FileID = CO.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE)
	) AS F
	LEFT JOIN
	CheckOut USING (FileID));

$BODY$
LANGUAGE sql;

--
--
--
-- get_latest_file_version
--
--
--
DROP FUNCTION get_latest_file_version(integer, integer);

CREATE OR REPLACE FUNCTION get_latest_file_version(IN user_id integer, IN file_id integer)
  RETURNS TABLE(fileid integer, name text, parentid integer, changesetid integer, created timestamp with time zone, size integer, data bytea, checkedout boolean, checkouttime timestamp with time zone, userid integer, action integer) AS
$BODY$
DECLARE
	is_checked_out boolean;
BEGIN
	SELECT true INTO is_checked_out FROM CheckOut WHERE CheckOut.FileID = file_id;

	IF (is_checked_out = TRUE AND ((SELECT CO.UserID FROM CheckOut CO WHERE CO.FileID = file_id) = user_id OR is_admin(user_id) = TRUE)) THEN
		RETURN QUERY
			SELECT
				F.FileID AS FileID,
				F.Name AS Name,
				F.ParentID AS ParentID,
				0,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				TRUE,	-- Checked_out
				CO.Time As ChechOutOrInTime,	-- CheckOutTime
				CO.UserID AS UserID,
				FI.Action AS Action
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
				F.Name AS Name,
				F.ParentID AS ParentID,
				CS.ChangesetID,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				FALSE,	-- Checked_in
				CS.Time As ChechOutOrInTime,	-- CheckIn time
				CS.UserID AS UserID,
				FI.Action AS Action
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


--
--
--
-- get_workcopy
--
--
--
DROP FUNCTION get_workcopy(integer, integer);

CREATE OR REPLACE FUNCTION get_workcopy(IN user_id integer, IN file_id integer)
  RETURNS TABLE(
		fileid integer, name text, parentid integer,
		created timestamp with time zone, size integer,
		data bytea, checkouttime timestamp with time zone,
		userid integer, action integer, details text) AS
$BODY$
	SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data as Data,
		CO.Time As ChechoutTime,
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
$BODY$
LANGUAGE sql;


--
--
--
-- check_in
--
--
--
DROP FUNCTION check_in(integer, integer[], text);

CREATE OR REPLACE FUNCTION check_in(IN user_id integer, IN file_ids integer[], IN checkin_comment text)
  RETURNS TABLE(cifileid integer, ciaction integer) AS
$BODY$
DECLARE
	NewChangesetID int;
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
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

	-- return table of FileID and Action pairs;

	RETURN QUERY (
		SELECT F.FileID, FI.ACTION
			FROM File F, FileInstance FI
			WHERE F.FileID = ANY(file_ids) AND FI.FileInstanceID = F.CheckedInInstanceID);
END;
$BODY$
  LANGUAGE plpgsql;

--
--
--
-- check_out
--
--
--
DROP FUNCTION check_out(integer, integer[]);

CREATE OR REPLACE FUNCTION check_out(user_id integer, file_ids integer[])
  RETURNS TABLE(cofileid integer) AS
$BODY$
DECLARE
	AlreadyCheckedOut integer;
	file_id integer;
	checkout_instance_id uuid;
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
	RETURN QUERY (SELECT FileID FROM CheckOut WHERE FileID = ANY(file_ids) AND UserID = user_id);
END
$BODY$
  LANGUAGE plpgsql;

--
--
--
-- delete_file
--
--
--
DROP FUNCTION delete_file(integer, integer);

CREATE OR REPLACE FUNCTION delete_file(user_id integer, file_id integer)
RETURNS integer AS
$BODY$
DECLARE
	checked_out boolean;
	checkout_user_id int;
	checked_out_instance_id uuid;
BEGIN
	-- CheckOut file if it has not been yet
	IF is_file_checkedout(file_id) = FALSE
	THEN
		checked_out := check_out(user_id, ARRAY[file_id]);
		IF (checked_out = FALSE)
		THEN
			RETURN 0;		-- Error, can't check out file
		END IF;
	END IF;

	-- get check out user id, file cout be checked out by other user, we must check it
	checkout_user_id := (SELECT UserID FROM CheckOut WHERE FileID = file_id);

	if (checkout_user_id <> user_id OR is_admin(user_id) = FALSE)
	THEN
		RETURN 0;			-- Error, user has no rights
	END IF;


	-- check if the file was not checked in yet (at least one time), then remove recors from fileinstance and file, checkout tables;
	checked_out_instance_id := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id AND CheckedInInstanceID IS NULL);

	if (checked_out_instance_id IS NOT NULL)
	THEN
		PERFORM (SELECT undo_changes(user_id, ARRAY[file_id]));
		RETURN 2;		-- File was deleted parmanently, there is no such file_id in tables!!!!!
	END IF;

	-- mark fileinstance action as deleted
	UPDATE FileInstance SET Action = 3 WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);
	RETURN 1;			-- FileInstance was marked as Action = Delete (3)
END
$BODY$
LANGUAGE plpgsql;


--
--
--
-- file_exists
--
--
--
CREATE OR REPLACE FUNCTION file_exists(file_id integer)
RETURNS boolean AS
$BODY$
	SELECT (COUNT(*) > 0) AS file_exists FROM File WHERE FileID = file_id AND Deleted = FALSE;
$BODY$
LANGUAGE sql;

--
--
--
-- files_exist
--
--
--
CREATE OR REPLACE FUNCTION files_exist(file_ids integer[])
RETURNS TABLE(fileid integer, fileexists boolean) AS
$BODY$
	SELECT IDS.FileID, F.Deleted = FALSE AND F.Deleted IS NOT NULL
	FROM	(
				(SELECT unnest(file_ids) AS FileID) AS IDS
					LEFT JOIN
				File AS F
					ON F.FileID = IDS.FileID
			);
$BODY$
LANGUAGE sql;
