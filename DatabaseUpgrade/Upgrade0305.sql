-- RPCT-2673
--
DROP FUNCTION public.get_changeset_details(integer, integer);

CREATE OR REPLACE FUNCTION api.get_changeset_details(
    session_key text,
	changeset_id integer)
	RETURNS SETOF dbchangesetdetails
	LANGUAGE 'plpgsql'
AS $BODY$
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Check changeset
	--
	IF (SELECT COUNT(*) FROM Changeset WHERE ChangesetID = changeset_id) = 0
	THEN
	    RAISE 'Changeset % not found.', changeset_id;
	END IF;

    RETURN QUERY
	    SELECT
		        -- DbChangeset
				CS.ChangesetID AS ChangesetID,
				CS.UserID AS UserID,
				U.Username AS Username,
				CS.Time AS CheckInTime,
				CS.Comment AS Comment,
				FI.Action AS Action,                -- Action defined in DbStruct.VcsItemAction
				-- DbChangesetObject
				0 AS ObjectType,                    -- 0 - file, 1 - signal
				F.FileID AS ObjectID,
				F.Name AS ObjectName,
				F.Name AS ObjectCaption,
				FI.Action AS ObjectAction,          -- Action defined in DbStruct.VcsItemAction
				F.ParentID::text AS ObjectParent,
				FI.MoveText AS filemovetext,
				FI.RenameText AS filerenametext				-- not implemented yet
				FROM
				Changeset CS,
				FileInstance FI,
				File F,
				Users U
				WHERE
				CS.ChangesetID = changeset_id AND
				FI.ChangesetID = CS.ChangesetID AND
				FI.FileID = F.FileID AND
				CS.UserID = U.UserID
				UNION ALL
		SELECT
		        -- DbChangeset
				CS.ChangesetID AS ChangesetID,
				CS.UserID AS UserID,
				U.Username AS Username,
				CS.Time AS CheckInTime,
				CS.Comment AS Comment,
				SI.Action AS Action,                -- Action defined in DbStruct.VcsItemAction
				-- DbChangesetObject
				1 AS ObjectType,                    -- 0 - file, 1 - signal
				S.SignalID AS ObjectID,
				SI.AppSignalID AS ObjectName,
				SI.CustomAppSignalID AS ObjectCaption,
				SI.Action AS ObjectAction,          -- Action defined in DbStruct.VcsItemAction
				SI.EquipmentID AS ObjectParent,
				'' AS filemovetext,					-- signal does not have this feature
				'' AS filerenametext				-- signal does not have this feature
				FROM
				Changeset CS,
				SignalInstance SI,
				Signal S,
				Users U
				WHERE
				CS.ChangesetID = changeset_id AND
				SI.ChangesetID = CS.ChangesetID AND
				SI.SignalID = S.SignalID AND
				CS.UserID = U.UserID;
END
$BODY$;

-- get_file_history
--
DROP FUNCTION public.get_file_history(integer, integer);

CREATE OR REPLACE FUNCTION api.get_file_history(
    session_key text,
	file_id integer)
	RETURNS SETOF dbchangeset
	LANGUAGE 'plpgsql'
AS $BODY$
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

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
$BODY$;


-- FUNCTION: get_file_history_recursive
--
DROP FUNCTION public.get_file_history_recursive(integer, integer);

CREATE OR REPLACE FUNCTION api.get_file_history_recursive(
    session_key text,
	parent_id integer)
	RETURNS SETOF dbchangeset
	LANGUAGE 'plpgsql'
AS $BODY$
DECLARE
    user_id integer;
	tree_files_ids integer[];
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    user_id := user_api.current_user_id(session_key);

    tree_files_ids :=
	    array(
		    SELECT SQ.FileID FROM (
			    (WITH RECURSIVE files(FileID, ParentID) AS (
				        SELECT FL1.FileID, FL1.ParentID FROM get_file_list(user_id, parent_id) AS FL1
					UNION ALL
					    SELECT FL2.FileID, FL2.ParentID FROM Files, get_file_list(user_id, files.FileID) FL2
					)
					SELECT * FROM files)
				UNION ALL
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
			0 AS Action
		FROM
		    FileInstance, Changeset, Users
		WHERE
		    FileInstance.FileID = ANY(tree_files_ids) AND
			FileInstance.ChangesetID =  Changeset.ChangesetID AND
			Changeset.UserID = Users.UserID
		ORDER BY
		    Changeset.ChangesetID DESC;
END
$BODY$;
