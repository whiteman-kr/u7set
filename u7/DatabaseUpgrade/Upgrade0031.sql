-------------------------------------------------------------------------------
--
--		get_file_info
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_file_info(IN user_id integer, file_ids integer[])
  RETURNS TABLE(fileid integer, deleted boolean, name text, parentid integer, created timestamp with time zone, fileinstanceid uuid, changesetid integer, size integer, instancecreated timestamp with time zone, changesettime timestamp with time zone, userid integer, checkedout boolean, action integer) AS
$BODY$

(SELECT
	F.FileID AS FileID,
	F.Deleted AS Deleted,
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
		F.Deleted AS Deleted,
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
		F.FileID = ANY(file_ids) AND
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
		F.Deleted AS Deleted,
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
		F.FileID = ANY(file_ids) AND
		F.CheckedOutInstanceID = FI.FileInstanceID AND
		F.FileID = FI.FileID AND
		F.FileID = CO.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE)
	) AS F
	LEFT JOIN
	CheckOut USING (FileID))
ORDER BY Name;

$BODY$
  LANGUAGE sql;
