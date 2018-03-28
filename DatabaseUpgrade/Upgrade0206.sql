--
--  RPCT-2036 - get_latest_file_tree_version optimization
--  1. Add api.get_latest_file_version with input array param
--  2. Do not call get_file_list for files without children
--

CREATE OR REPLACE FUNCTION api.get_latest_file_version(
    session_key text,
    file_ids integer[])
  RETURNS SETOF dbfile AS
$BODY$
DECLARE
	current_user_id integer;
	is_user_an_admin boolean;
	is_checked_out boolean;
	fid integer;
	file_result DbFile;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

	current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);		

	--
    --
  	FOREACH fid IN ARRAY file_ids
  	LOOP
        	-- set is_checked_out variable
	        --
	        SELECT true INTO is_checked_out FROM CheckOut WHERE CheckOut.FileID = fid;

	        -- get file for fid
	        --
	        IF (is_checked_out = TRUE AND ((SELECT CO.UserID FROM CheckOut CO WHERE CO.FileID = fid) = current_user_id OR is_user_an_admin = TRUE)) 
	        THEN
			SELECT
				F.FileID AS FileID,
				F.Deleted AS Deleted,
				F.Name AS Name,
				F.ParentID AS ParentID,
				0,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				TRUE,	-- Checked_out
				CO.Time As ChechOutOrInTime,	-- CheckOutTime
				CO.UserID AS UserID,
				FI.Action AS Action,
				FI.Details::text AS Details
			INTO 
				file_result
			FROM
				File F, FileInstance FI, Checkout CO
			WHERE
				F.FileID = fid AND
				FI.FileInstanceID = F.CheckedOutInstanceID AND
				CO.FileID = fid AND
				(current_user_id = CO.UserID OR is_user_an_admin = TRUE);
		ELSE
			SELECT
				F.FileID AS FileID,
				F.Deleted AS Deleted,
				F.Name AS Name,
				F.ParentID AS ParentID,
				CS.ChangesetID,
				F.Created AS Created,
				length(FI.Data) AS Size,
				FI.Data as Data,
				FALSE,	-- Checked_in
				CS.Time As ChechOutOrInTime,	-- CheckIn time
				CS.UserID AS UserID,
				FI.Action AS Action,
				FI.Details::text AS Details
			INTO 
	                	file_result				
			FROM
				File F, FileInstance FI, Changeset CS
			WHERE
				F.FileID = fid AND
				FI.FileInstanceID = F.CheckedInInstanceID AND
				CS.ChangesetID = FI.ChangesetID;
	        END IF;
  		
	  	RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_latest_file_tree_version(
    session_key text,
    file_id integer)
  RETURNS SETOF dbfile AS
$BODY$
DECLARE
	is_checked_out boolean;
	file_ids integer[];
	fid integer;
	file_result DbFile;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

	-- Get all files list
	--
	file_ids := array(
					SELECT SQ.FileID FROM (
						(WITH RECURSIVE files(FileID, ParentID) AS (
								SELECT FileID, ParentID FROM api.get_file_list(session_key, file_id) WHERE Deleted = FALSE AND Action <> 3
							UNION ALL
								SELECT FL.FileID, FL.ParentID FROM Files, api.get_file_list(session_key, files.FileID) FL WHERE FL.Deleted = FALSE AND Action <> 3 AND EXISTS (SELECT * FROM File WHERE File.ParentID = Files.FileID)
							)
							SELECT * FROM files)
						UNION
							SELECT FileID, ParentID FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = file_id)) WHERE FileID = file_id
					) SQ
					ORDER BY SQ.FileID
				);

	-- Read files latest version
	--
    RETURN QUERY 
        SELECT * FROM api.get_latest_file_version(session_key, file_ids);

END
$BODY$
  LANGUAGE plpgsql;
