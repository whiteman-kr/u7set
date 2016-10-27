--
--  RPCT-1222
--
CREATE TYPE dbchangeset AS
	(
	changesetid integer, 
	userid integer, 
	username text, 
	checkintime timestamp with time zone, 
	comment text, 
	action integer
	);


CREATE TYPE dbchangesetdetails AS
	(
	-- DbChangeset
 	changesetid 		integer, 
 	userid 			integer, 
 	username 		text, 
 	checkintime 		timestamp with time zone, 
 	comment 		text, 
 	action			integer,
 	-- DbChangesetObject
 	objecttype		integer,	-- 0 - file, 1 - signal
 	objectid		integer,	-- File.FileID for file and Signal.SignalID for Signal
 	objectname		text,		-- Filename or AppSignalID
 	objectcaption		text,		-- Can be Caption for schema, EquipmentID for Device, Caption for Signal
 	objectaction		integer,	-- Action defined in DbStruct.VcsItemAction
 	objectparent		text		-- Parent file, can be EquipmentID for Devices 	
 	);


CREATE OR REPLACE FUNCTION get_changeset_details(user_id integer, changeset_id integer )
	RETURNS SETOF DbChangesetDetails AS
$BODY$
BEGIN
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
                F.ParentID::text AS ObjectParent
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
                SI.Caption AS ObjectCaption,
                SI.Action AS ObjectAction,          -- Action defined in DbStruct.VcsItemAction
                SI.EquipmentID AS ObjectParent
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
$BODY$
LANGUAGE plpgsql;


-- Drop previous implementation of get_file_history
--
DROP FUNCTION get_file_history(integer, integer);

-- implementation of get_file_history(user_id integer, file_id integer)
--
CREATE OR REPLACE FUNCTION get_file_history(user_id integer, file_id integer)
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
DROP FUNCTION get_file_history_recursive(integer, integer);

CREATE OR REPLACE FUNCTION get_file_history_recursive(user_id integer, parent_id integer)
	RETURNS SETOF DbChangeset AS
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
$BODY$
LANGUAGE plpgsql;
