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

