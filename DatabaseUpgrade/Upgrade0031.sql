-------------------------------------------------------------------------------
--
--		get_file_info
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_file_info(IN user_id integer, file_ids integer[])
  RETURNS
	TABLE(
		fileid integer, deleted boolean, name text, parentid integer, created timestamp with time zone,
		fileinstanceid uuid, changesetid integer, size integer, instancecreated timestamp with time zone,
		changesettime timestamp with time zone, userid integer, checkedout boolean, action integer, details text) AS
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


-------------------------------------------------------------------------------
--
--		get_file_history
--
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION get_file_history(file_id integer)
RETURNS TABLE(changesetid int, userid int, checkintime timestamp with time zone, comment text, action int) AS
$BODY$
BEGIN

	RETURN QUERY
		SELECT
			Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			FileInstance.Action AS Action
		FROM
			FileInstance, Changeset
		WHERE
			FileInstance.FileID = file_id AND
			FileInstance.ChangesetID =  Changeset.ChangesetID
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------------------------------------------------------
--
--		get_specific_copy
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_specific_copy(IN user_id integer, IN file_id integer, IN changeset_id integer)
RETURNS
	TABLE(
		fileid integer, name text, parentid integer, created timestamp with time zone,
		size integer, data bytea, checkintime timestamp with time zone, userid integer,
		action integer, details text) AS
$BODY$
	SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data as Data,
		CS.Time As ChechInTime,
		CS.UserID AS UserID,
		FI.Action AS Action,
		FI.Details::text AS Details
	FROM
		File F, FileInstance FI, Changeset CS
	WHERE
		F.FileID = file_id AND
		FI.FileID = file_id AND
		FI.ChangesetId = changeset_id AND
		CS.ChangesetID = changeset_id;
$BODY$
LANGUAGE sql;
