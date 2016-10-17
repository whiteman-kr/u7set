-------------------------------------------------------------------------------
--		RPCT-1186 -- Username is added to return result of get_file_history
--
--		get_file_history
--
-------------------------------------------------------------------------------
DROP FUNCTION public.get_file_history(integer);


CREATE OR REPLACE FUNCTION public.get_file_history(IN file_id integer)
	RETURNS TABLE(changesetid integer, userid integer, username text, checkintime timestamp with time zone, comment text, action integer) AS
$BODY$
BEGIN
	RETURN QUERY
		SELECT
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
			Changeset.UserID = Users.Userid
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
	LANGUAGE plpgsql;

