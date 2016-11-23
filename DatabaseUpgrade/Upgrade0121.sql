-- RPCT-1277

-------------------------------------------------------------------------------
--
--		get_specific_copy
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_specific_copy(user_id integer, file_id integer, changeset_id integer)
  RETURNS DbFile AS
$BODY$
DECLARE
	result DbFile;
    actual_changeset_id integer;
		
BEGIN
    actual_changeset_id := (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = file_id AND ChangesetID <= changeset_id);

	SELECT
		F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		CS.ChangesetID AS ChangesetID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data as Data,
		(SELECT count(*) > 0 FROM CheckOut WHERE FileID = file_id) AS CheckedOut,
		--(SELECT Time FROM CheckOut WHERE FileID = file_id) AS CheckOutTime,
		CS.Time As ChechInTime,
		CS.UserID AS UserID,
		FI.Action AS Action,
		FI.Details::text AS Details
	INTO
		result
	FROM
		File F, FileInstance FI, Changeset CS
	WHERE
		F.FileID = file_id AND
		FI.FileID = file_id AND
		FI.ChangesetId = actual_changeset_id AND
		CS.ChangesetID = actual_changeset_id;

	IF (result IS NULL) THEN
		RAISE EXCEPTION 'Cannot find file copy for changeset (FileID: %, ChangesetID: %)', file_id, changeset_id;
	END IF;

	RETURN result;
END;
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		get_specific_copy
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_specific_copy(user_id integer, file_id integer, changeset_data timestamp with time zone)
  RETURNS DbFile AS
$BODY$
DECLARE
	result DbFile;
	actual_changeset_id integer;
		
BEGIN
	actual_changeset_id := (SELECT max(ChangesetID) FROM FileInstance WHERE FileID = file_id AND created <= changeset_data);

	SELECT * INTO result FROM get_specific_copy(user_id, file_id, actual_changeset_id);

	RETURN result;
END;
$BODY$
LANGUAGE plpgsql;
