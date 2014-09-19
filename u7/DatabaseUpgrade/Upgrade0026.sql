DROP FUNCTION getfilelist(integer, text);

-- функция должна возращать список файлов для пользовательзователя user_id.
-- 1. В список должны включаться все зарегистрированные файлы которые находяться в состоянии CheckedIn.
-- 2. Если файл взят на редактирование (CheckedOut) и был хоть один раз зарегистрирован он тоже включается (состояние CheckedOut).
-- 3 Если файл еще не был зарегистрирован, он включается в список только если пользователь является владельцем файла или пользователь администратор (состояние CheckedOut).
-- В  функции два зхапроса, объедененных через UNION,  в первом запросе выполняется LEFT JOIN с таблицей Changeset,
-- воторой запрос объеденяется с таблицей CheckeOut, bp 'nb[ hfpys[ nf,kbw ,thtnmcz UserID (Кто делал CheckIn и кто делал CheckOut)
--
CREATE OR REPLACE FUNCTION get_file_list(user_id integer, parent_id integer, file_mask text)
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
		checkedout boolean
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
	F.ChangesetID IS NULL AS CheckedOut
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
		FI.Created AS InstanceCreated
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
	F.ChangesetID IS NULL AS CheckedOut
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
		FI.Created AS InstanceCreated
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


DROP FUNCTION addfile(integer, text, integer, integer, bytea);

CREATE OR REPLACE FUNCTION add_file(user_id integer, file_name text, parent_id integer, file_data bytea)
RETURNS integer AS
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

	RETURN newfileid;
END
$BODY$
LANGUAGE plpgsql;


DROP FUNCTION getworkcopy(integer);

CREATE OR REPLACE FUNCTION get_workcopy(user_id integer, file_id integer)
RETURNS
	TABLE(
		fileid integer,
		name text,
		parentid integer,
		created timestamp with time zone,
		size integer,
		data bytea,
		checkouttime timestamp with time zone,
		userid integer)
	 AS
$BODY$
	SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data as Data,
		CO.Time As ChechoutTime,
		CO.UserID AS UserID
	FROM
		File F, FileInstance FI, Checkout CO
	WHERE
		F.FileID = file_id AND
		FI.FileInstanceID = F.CheckedOutInstanceID AND
		CO.FileID = file_id AND
		(user_id = CO.UserID OR (SELECT is_admin(user_id)) = TRUE);
$BODY$
LANGUAGE sql;



DROP FUNCTION setworkcopy(integer, bytea);

CREATE OR REPLACE FUNCTION set_workcopy(user_id integer, file_id integer, file_data bytea)
RETURNS INT AS
$BODY$
DECLARE
	user_allowed int;
	inst_file_id int;
BEGIN
	SELECT count(*) INTO user_allowed
		FROM CheckOut WHERE FileID = file_id AND UserID = user_id;

	IF (user_allowed = 0 AND is_admin(user_id) = FALSE) THEN
		RAISE 'User is not allowed to set workcopy for file_id %', file_id;
	END IF;

	UPDATE FileInstance SET Size = length(file_data), Data = file_data
		WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id)
		RETURNING FileID INTO inst_file_id;

	IF (inst_file_id <> file_id) THEN
		RAISE 'DATABASE CRITICAL ERROR, FileID in File and FileInstance tables is different! %', file_id;
	END IF;

	RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;


DROP FUNCTION undochanges(integer[], integer);

CREATE OR REPLACE FUNCTION undo_changes(user_id integer, file_ids integer[])
RETURNS integer AS
$BODY$
DECLARE
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
BEGIN
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

	-- update table file, set CheckedOutIntsnceID to NULL
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(file_ids);

	-- Delete from file instance all these files
	DELETE FROM FileInstance WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	-- Delete all check outs
	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);


	-- if column File.CheckedInIntsnceID is NULL then this file was not checked in, and we have to remove it from the table
	DELETE FROM File WHERE FileID = ANY(file_ids) AND CheckedInInstanceID IS NULL;

	RETURN 0;
END;
$BODY$
  LANGUAGE plpgsql;


DROP FUNCTION checkin(integer[], integer, text);

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

	--
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, TRUE) RETURNING ChangesetID INTO NewChangesetID;

	UPDATE File SET CheckedInInstanceID = CheckedOutInstanceID WHERE FileID = ANY(file_ids);
	UPDATE File SET CheckedOutInstanceID = NULL WHERE FileID = ANY(file_ids);

	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);

	RETURN NewChangesetID;
END;
$BODY$
  LANGUAGE plpgsql;


DROP FUNCTION checkout(integer[], integer);

CREATE OR REPLACE FUNCTION check_out(user_id integer, file_ids integer[])
  RETURNS boolean AS
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
		RETURN FALSE;
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

	RETURN TRUE;
END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION get_latest_file_version(user_id integer, file_id integer)
RETURNS
	TABLE(
		fileid integer,
		name text,
		parentid integer,
		changesetid integer,
		created timestamp with time zone,
		size integer,
		data bytea,
		checkedout boolean,
		checkouttime timestamp with time zone,
		userid integer
	) AS
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
				CO.UserID AS UserID
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
				CS.UserID AS UserID
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


CREATE OR REPLACE FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text)
  RETURNS integer AS
$BODY$
DECLARE
	hc_fileid int;
	fileid_lenght int;
	new_filename text;
	new_fileid int;
BEGIN
	-- TO DO: Check user right here

	-- generate filename
	SELECT * INTO new_filename FROM uuid_generate_v1();
	SELECT octet_length(new_filename) INTO fileid_lenght;

	new_filename := 'device-' || new_filename || file_extension;	-- smthng like: device-5be363ac-3c02-11e4-9de8-3f84f459cb27.hsystem

	-- add new file
	SELECT * INTO new_fileid FROM add_file(user_id, new_filename, parent_id, file_data);

	RETURN new_fileid;
END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION file_has_children(user_id integer, file_id integer)
RETURNS integer AS
$BODY$

SELECT SUM(CNT)::integer AS ChildCount FROM
	((	-- All checked in now
	SELECT
		COUNT(*) AS CNT
	FROM
		File F,
		FileInstance FI
	WHERE
		F.ParentID = file_id AND
		F.CheckedInInstanceID = FI.FileInstanceID AND
		F.CheckedOutInstanceID IS NULL AND
		F.FileID = FI.FileID
	)
	UNION
	(	-- All CheckedOut by any user if user_id is administrator
	SELECT
		COUNT(*) AS CNT
	FROM
		File F,
		FileInstance FI,
		CheckOut CO
	WHERE
		F.ParentID = file_id AND
		F.CheckedOutInstanceID = FI.FileInstanceID AND
		F.FileID = FI.FileID AND
		F.FileID = CO.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE)
	)) AS SubQuery;

$BODY$
  LANGUAGE sql;


CREATE OR REPLACE FUNCTION is_file_checkedout(file_id integer)
RETURNS boolean AS
$BODY$
	SELECT (COUNT(*) > 0) AS checkedout FROM CheckOut WHERE FileID = file_id;
$BODY$
LANGUAGE sql;


CREATE OR REPLACE FUNCTION delete_file(user_id integer, file_id integer)
RETURNS boolean AS
$BODY$
DECLARE
	checked_out boolean;
	checkout_user_id int;
BEGIN
	-- CheckOut file if it has not been yet
	IF is_file_checkedout(file_id) = FALSE
	THEN
		checked_out := check_out(user_id, ARRAY[file_id]);
		IF (checked_out = FALSE)
		THEN
			RETURN FALSE;
		END IF;
	END IF;

	-- get check out user id, file cout be checked out by other user, we must check it
	checkout_user_id := (SELECT UserID FROM CheckOut WHERE FileID = file_id);

	if (checkout_user_id = user_id OR is_admin(user_id) = TRUE)
	THEN
		-- mark fileinstance action as deleted
		UPDATE FileInstance SET Action = 3 WHERE FileInstanceID = (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

		RETURN TRUE;
	ELSE
		-- oops, file was checked opu by other user, and user_id is not admin :(((
		RETURN FALSE;
	END IF;
END
$BODY$
LANGUAGE plpgsql;
