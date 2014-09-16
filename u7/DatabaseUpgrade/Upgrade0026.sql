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
