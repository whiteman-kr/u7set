CREATE OR REPLACE FUNCTION getworkcopy(file_id integer)
RETURNS TABLE (
	FileID integer,
	Name text,
	ParentID integer,
	Created timestamp with time zone,
	Size integer,
	Data bytea,
	CheckOutTime timestamp with time zone,
	UserID integer
) AS
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
		FI.FileID = file_id AND
		FI.ChangesetID IS NULL AND
		CO.FileID = file_id;
$BODY$
LANGUAGE sql;


CREATE OR REPLACE FUNCTION setworkcopy(file_id integer, file_data bytea)
  RETURNS integer AS
$BODY$
	UPDATE FileInstance SET Size = length(file_data), Data = file_data
		WHERE FileId = file_id AND ChangesetID IS NULL RETURNING FileID;
$BODY$
  LANGUAGE sql;
