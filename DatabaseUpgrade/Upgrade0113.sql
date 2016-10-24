--
--  RPCT-1205
--
             
-- Create index for Fileinstance.FileID
-- It's speeds up get_file_list and recursive request for getting files tree
--
CREATE INDEX fileinstance_index_fileid
ON fileinstance USING btree (fileid);


-- Drop previous implementation of get_file_history
--
DROP FUNCTION get_file_history(integer);

-- implementation of get_file_history(user_id integer, file_id integer)
--
CREATE OR REPLACE FUNCTION get_file_history(user_id integer, file_id integer)
	RETURNS TABLE(changesetid integer, userid integer, username text, checkintime timestamp with time zone, comment text, action integer) AS
$BODY$
BEGIN
	RETURN QUERY
		SELECT DISTINCT
			Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Users.Username AS Username,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			FileInstance.Action AS Action
		FROM
			FileInstance, Changeset, Users
		WHERE
			FileInstance.FileID = file_id AND
			FileInstance.ChangesetID =  Changeset.ChangesetID AND
			Changeset.UserID = Users.UserID
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
LANGUAGE plpgsql;


-- implementation of get_file_history_recursive(user_id integer, parent_id integer)
--
CREATE OR REPLACE FUNCTION get_file_history_recursive(user_id integer, parent_id integer)
	RETURNS TABLE(changesetid integer, userid integer, username text, checkintime timestamp with time zone, comment text, action integer) AS
$BODY$
DECLARE
	tree_files_ids integer[];
BEGIN
	tree_files_ids := 
		array(
			SELECT SQ.FileID FROM (
				(WITH RECURSIVE files(FileID, ParentID) AS (
						SELECT FL1.FileID, FL1.ParentID FROM get_file_list(user_id, parent_id) AS FL1
					UNION ALL
						SELECT FL2.FileID, FL2.ParentID FROM Files, get_file_list(user_id, files.FileID) FL2
					)
					SELECT * FROM files)
				UNION
					SELECT FL3.FileID, FL3.ParentID FROM get_file_list(user_id, (SELECT ParentID FROM File WHERE File.FileID = parent_id)) AS FL3 WHERE FL3.FileID = parent_id
			) SQ
		);

	RETURN QUERY
		SELECT DISTINCT
			Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Users.Username AS Username,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			FileInstance.Action AS Action
		FROM
			FileInstance, Changeset, Users
		WHERE
			FileInstance.FileID = ANY(tree_files_ids) AND
			FileInstance.ChangesetID =  Changeset.ChangesetID AND
			Changeset.UserID = Users.UserID
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
LANGUAGE plpgsql;

