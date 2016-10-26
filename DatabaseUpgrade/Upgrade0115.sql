--
--  RPCT-1225
--

-- implementation of get_history(user_id integer)
--
CREATE OR REPLACE FUNCTION get_history(user_id integer)
	RETURNS SETOF DbChangeset AS
$BODY$
BEGIN
	RETURN QUERY
		SELECT DISTINCT
			Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Users.Username AS Username,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			1 AS Action
		FROM
			Changeset, Users
		WHERE
			Changeset.UserID = Users.UserID
		ORDER BY
			Changeset.ChangesetID DESC;
END
$BODY$
LANGUAGE plpgsql;