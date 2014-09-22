DROP FUNCTION get_file_list(integer, integer, text);

CREATE OR REPLACE FUNCTION get_file_list(IN user_id integer, IN parent_id integer, IN file_mask text)
RETURNS
	TABLE(
		fileid integer,
		name text,
		parentid integer,
		created timestamp with time zone,
		fileinstanceid uuid,
		changesetid integer,
		size integer,
		instancecreated timestamp with time zone,
		changesettime timestamp with time zone,
		userid integer,
		checkedout boolean,
		action integer
	) AS
$BODY$

(SELECT
	F.FileID AS FileID,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.Created AS Created,
	F.FileInstanceID AS FileInstanceID,
	F.ChangesetID AS ChangesetID,
	F.Size AS Size,
	F.InstanceCreated AS InstanceCreated,
	Changeset.time AS ChangesetTime,
	Changeset.UserID AS UserID,
	F.ChangesetID IS NULL AS CheckedOut,
	F.Action AS Action
FROM
	-- All checked in now
	(SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action
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
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.Created AS Created,
	F.FileInstanceID AS FileInstanceID,
	F.ChangesetID AS ChangesetID,
	F.Size AS Size,
	F.InstanceCreated AS InstanceCreated,
	CheckOut.time AS ChangesetTime,
	CheckOut.UserID AS UserID,
	F.ChangesetID IS NULL AS CheckedOut,
	F.Action AS Action
FROM
	-- All CheckedOut by any user if user_id is administrator
	(SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Action AS Action
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


DROP FUNCTION get_workcopy(integer, integer);

CREATE OR REPLACE FUNCTION get_workcopy(IN user_id integer, IN file_id integer)
  RETURNS TABLE(fileid integer, name text, parentid integer, created timestamp with time zone, size integer, data bytea, checkouttime timestamp with time zone, userid integer, action integer) AS
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
		FI.Action AS Action
	FROM
		File F, FileInstance FI, Checkout CO
	WHERE
		F.FileID = file_id AND
		FI.FileInstanceID = F.CheckedOutInstanceID AND
		CO.FileID = file_id AND
		(user_id = CO.UserID OR (SELECT is_admin(user_id)) = TRUE);
$BODY$
LANGUAGE sql;


DROP FUNCTION check_in(integer, integer[], text);

CREATE OR REPLACE FUNCTION check_in(user_id integer, file_ids integer[], checkin_comment text)
  RETURNS integer AS
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
			RETURN FALSE;
		END IF;

		-- Check if the file can be undo by this user_id
		SELECT UserId INTO file_user FROM CheckOut WHERE FileID = file_id;

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
			RETURN FALSE;
		END IF;
	END LOOP;

	-- Add new record to Changeset
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, TRUE) RETURNING ChangesetID INTO NewChangesetID;

	-- Remove deleted records from File table, only if this file was not checkedin before
	DELETE FROM File
		WHERE
			FileID = ANY(file_ids) AND
			CheckedInInstanceID IS NULL AND
			(SELECT Action FROM FileInstance WHERE FileInstanceID = CheckedOutInstanceID) = 3;

	-- Set File.Deleted flag if action in FileInstance is Deleted
	UPDATE File SET Deleted = TRUE
		WHERE
			FileID = ANY(file_ids) AND
			(SELECT Action FROM FileInstance WHERE FileInstanceID = CheckedOutInstanceID) = 3;

	-- Set CheckedInInstance to current CheckedOutInstance, and set CheckedOutInstanceID to NULL
	UPDATE File SET CheckedInInstanceID = CheckedOutInstanceID WHERE FileID = ANY(file_ids);
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(file_ids);

	-- Update FileInstance, set it's ChangesetID
	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	-- Remove CheckOut roecords
	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);

	RETURN NewChangesetID;
END;
$BODY$
  LANGUAGE plpgsql;
