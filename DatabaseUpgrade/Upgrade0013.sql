CREATE OR REPLACE FUNCTION getfilelist(filemask text)
RETURNS TABLE (
	FileID integer,
	Name text,
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
