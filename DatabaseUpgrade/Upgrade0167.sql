-- Fixining error, deleted but not checked in file remains in the result
--
CREATE OR REPLACE FUNCTION get_latest_file_tree_version(IN user_id integer, IN file_id integer)
	RETURNS SETOF DbFile AS
$BODY$
DECLARE
	is_checked_out boolean;
	file_ids integer[];
	fid integer;
	file_result DbFile;
BEGIN
	-- Get all files list
	file_ids := array(
					SELECT SQ.FileID FROM (
						(WITH RECURSIVE files(FileID, ParentID) AS (
								SELECT FileID, ParentID FROM get_file_list(user_id, file_id) WHERE Deleted = FALSE AND Action <> 3
							UNION ALL
								SELECT FL.FileID, FL.ParentID FROM Files, get_file_list(user_id, files.FileID) FL WHERE FL.Deleted = FALSE AND Action <> 3
							)
							SELECT * FROM files)
						UNION
							SELECT FileID, ParentID FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE FileID = file_id)) WHERE FileID = file_id
					) SQ
					ORDER BY SQ.FileID
				);

	-- Read files latest version
	FOREACH fid IN ARRAY file_ids
	LOOP
		file_result := get_latest_file_version(user_id, fid);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;
