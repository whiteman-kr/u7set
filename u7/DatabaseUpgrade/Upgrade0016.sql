DROP FUNCTION addfile(int, text, int, bytea);

CREATE OR REPLACE FUNCTION addfile(userid integer, filename text, parent_id integer, filesize integer, filedata bytea)
RETURNS integer AS
$BODY$
DECLARE
	exists int;
	newfileid int;
BEGIN
	SELECT count(*) INTO exists FROM File WHERE Name=filename AND ParentID=parent_id AND Deleted=false;
	IF (exists > 0) THEN
		RAISE 'Duplicate file name: %', filename USING ERRCODE = 'unique_violation';
	END IF;

	INSERT INTO File (Name, ParentID, Deleted)
		VALUES (filename, parent_id, false) RETURNING FileID INTO newfileid;

	INSERT INTO CheckOut (UserID, FileID)
		VALUES (userid, newfileid);

	INSERT INTO FileInstance (FileID, Size, Data, Action)
		VALUES (newfileid, filesize, filedata, 1);

	RETURN newfileid;
END
$BODY$
LANGUAGE plpgsql;


DROP FUNCTION getfilelist(text);

CREATE OR REPLACE FUNCTION getfilelist(parent_id integer, filemask text)
RETURNS TABLE (
	FileID integer,
	Name text,
	ParentID integer,
	Created timestamp with time zone,
	FileInstanceID uuid,
	ChangesetID integer,
	Size integer,
	InstanceCreated timestamp with time zone,
	Sequence integer,
	ChangesetTime timestamp with time zone,
	UserID integer,
	CheckedOut boolean
) AS
$BODY$
SELECT
	F.FileID AS FileID,
	F.Name AS Name,
	F.ParentID AS ParentID,
	F.Created AS Created,
	F.FileInstanceID AS FileInstanceID,
	F.ChangesetID AS ChangesetID,
	F.Size AS Size,
	F.InstanceCreated AS InstanceCreated,
	F.Sequence AS Sequence,
	Changeset.time AS ChangesetTime,
	Changeset.UserID AS UserID,
	F.ChangesetID IS NULL AS CheckedOut
FROM
	(SELECT
		F.FileID AS FileID,
		F.Name AS Name,
		F.ParentID AS ParentID,
		F.Created AS Created,
		FI.FileInstanceID AS FileInstanceID,
		FI.ChangesetID AS ChangesetID,
		length(FI.data) AS Size,
		FI.Created AS InstanceCreated,
		FI.Sequence AS Sequence
	FROM
		File F,
		FileInstance FI,
		(SELECT FileID, max(Sequence) AS MaxSeq FROM FileInstance GROUP BY FileID) AS TFI
	WHERE
		F.ParentID = parent_id AND
		F.FileID = FI.FileID AND
		F.FileID = TFI.FileID AND
		FI.Sequence = TFI.MaxSeq AND
		F.Name ILIKE filemask
	ORDER BY F.name) AS F
	LEFT JOIN
	Changeset USING (ChangesetID)
ORDER BY Name;
$BODY$
  LANGUAGE sql;
