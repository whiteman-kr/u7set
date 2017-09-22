-- Fixining error, deleted but not checked in file remains in the result
--
DROP FUNCTION public.get_latest_file_tree_version(integer, integer);

CREATE OR REPLACE FUNCTION api.get_latest_file_tree_version(session_key text, IN file_id integer)
	RETURNS SETOF DbFile AS
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
								SELECT FL.FileID, FL.ParentID FROM Files, api.get_file_list(session_key, files.FileID) FL WHERE FL.Deleted = FALSE AND Action <> 3
							)
							SELECT * FROM files)
						UNION
							SELECT FileID, ParentID FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = file_id)) WHERE FileID = file_id
					) SQ
					ORDER BY SQ.FileID
				);

	-- Read files latest version
	--
	FOREACH fid IN ARRAY file_ids
	LOOP
		file_result := api.get_latest_file_version(session_key, fid);
		RETURN NEXT file_result;
	END LOOP;

	RETURN;
END
$BODY$
LANGUAGE plpgsql;

-- Move get_checked_out_filesto api schema, fix call get_file_info
-- 
DROP FUNCTION public.get_checked_out_files(integer, integer[]);

CREATE OR REPLACE FUNCTION api.get_checked_out_files(session_key text, parent_file_ids integer[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    current_user_id integer;
	parent_id int;
	checked_out_ids integer[];
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Get files list
    --
	FOREACH parent_id IN ARRAY parent_file_ids
	LOOP
		-- get all checked out files for parents, including parent
		checked_out_ids := array_cat(
			array(
				SELECT SQ.FileID
				FROM (
					(WITH RECURSIVE files(FileID, ParentID, CheckedOut) AS (
							SELECT FileID, ParentID, CheckedOut FROM api.get_file_list(session_key, parent_id)
						UNION ALL
							SELECT FL.FileID, FL.ParentID, FL.CheckedOut FROM Files, api.get_file_list(session_key, files.FileID) FL
						)
						SELECT * FROM files)
					UNION
						SELECT FileID, ParentID, CheckedOut FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = parent_id)) WHERE FileID = parent_id
				) SQ
				WHERE CheckedOut = TRUE
			), checked_out_ids);
	END LOOP;

	IF (array_length(checked_out_ids, 1) = 0)
	THEN
		RETURN;
	END IF;

	-- Remove same identifiers, Ids will be sorted
	--
	checked_out_ids := (SELECT * FROM uniq(sort(checked_out_ids)));

	-- Return result
	--
	RETURN QUERY SELECT * FROM api.get_file_info(session_key, checked_out_ids);
END;
$BODY$
LANGUAGE plpgsql;

-- Move get_file_list to api schema, leave ald version for cmaptibility whith other (non api) functions, drop this func in future
--
--DROP FUNCTION public.get_file_list(integer, integer);	

CREATE OR REPLACE FUNCTION api.get_file_list(
    session_key text,
    parent_id integer)
    RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    current_user_id integer;
    is_user_an_admin boolean;
BEGIN    
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

    current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);		

	-- All checked in now
	--
	RETURN QUERY
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            CS.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            false AS CheckedOut,
            CS.Time AS CheckOutTime,
            CS.UserID AS USerID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            Changeset CS
        WHERE
            F.ParentID = parent_id AND
            F.CheckedInInstanceID = FI.FileInstanceID AND
            F.CheckedOutInstanceID IS NULL AND
            --F.FileID = FI.FileID AND 	-- F.CheckedInInstanceID = FI.FileInstanceID, it is soppose that F.CheckedInInstanceID pointed to the right FileID
            CS.ChangesetID = FI.ChangesetID
        )
    UNION
        -- All CheckedOut by any user if user_id is administrator
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            FI.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            true AS CheckedOut,
            CO.Time AS CheckOutTime,
            CO.UserID AS UserID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            CheckOut CO
        WHERE
            F.ParentID = parent_id AND
            FI.FileInstanceID = F.CheckedOutInstanceID AND
            --F.FileID = FI.FileID AND		-- it is done by (F.CheckedOutInstanceID = FI.FileInstanceID)
            CO.FileID = F.FileID AND
            (CO.UserID = current_user_id OR is_user_an_admin = TRUE)
        )
    UNION
        -- All CheckedOut by somebody else and was checked in it least one time
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            FI.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            true AS CheckedOut,
            CO.Time AS CheckOutTime,
            CO.UserID AS UserID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            CheckOut CO
        WHERE
            F.ParentID = parent_id AND
            FI.FileInstanceID = F.CheckedInInstanceID AND
            --F.FileID = FI.FileID AND			-- it is done by (FI.FileInstanceID = F.CheckedInInstanceID)
            CO.FileID = F.FileID AND
            --F.CheckedInInstanceID IS NOT NULL AND		-- done by FI.FileInstanceID = F.CheckedInInstanceID,
            (CO.UserID <> current_user_id AND is_user_an_admin = FALSE)
        );
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_file_list(
    session_key text,
    parent_id integer,
    file_mask text)
RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    current_user_id integer;
    is_user_an_admin boolean;
BEGIN    
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

    current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);		

	-- All checked in now
	--
	RETURN QUERY
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            CS.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            false AS CheckedOut,
            CS.Time AS CheckOutTime,
            CS.UserID AS USerID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            Changeset CS
        WHERE
            F.ParentID = parent_id AND
            F.CheckedInInstanceID = FI.FileInstanceID AND
            F.CheckedOutInstanceID IS NULL AND
            --F.FileID = FI.FileID AND 	-- F.CheckedInInstanceID = FI.FileInstanceID, it is soppose that F.CheckedInInstanceID pointed to the right FileID
            CS.ChangesetID = FI.ChangesetID AND
            F.Name ILIKE file_mask
        )
        UNION
        -- All CheckedOut by any user if user_id is administrator
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            FI.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            true AS CheckedOut,
            CO.Time AS CheckOutTime,
            CO.UserID AS UserID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            CheckOut CO
        WHERE
            F.ParentID = parent_id AND
            FI.FileInstanceID = F.CheckedOutInstanceID AND
            --F.FileID = FI.FileID AND		-- it is done by (F.CheckedOutInstanceID = FI.FileInstanceID)
            CO.FileID = F.FileID AND
            (CO.UserID = current_user_id OR is_user_an_admin = TRUE) AND
            F.Name ILIKE file_mask
        )
        UNION
        -- All CheckedOut by somebody else and was checked in it least one time
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            FI.ChangesetID AS ChangesetID,
            F.Created AS Created,
            length(FI.data) AS Size,
            true AS CheckedOut,
            CO.Time AS CheckOutTime,
            CO.UserID AS UserID,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            CheckOut CO
        WHERE
            F.ParentID = parent_id AND
            FI.FileInstanceID = F.CheckedInInstanceID AND
            --F.FileID = FI.FileID AND			-- it is done by (FI.FileInstanceID = F.CheckedInInstanceID)
            CO.FileID = F.FileID AND
            --F.CheckedInInstanceID IS NOT NULL AND		-- done by FI.FileInstanceID = F.CheckedInInstanceID,
            (CO.UserID <> current_user_id AND is_user_an_admin = FALSE) AND
            F.Name ILIKE file_mask
        );
END
$BODY$
LANGUAGE plpgsql;


-- Move get_latest_file_version to api schema
--
DROP FUNCTION public.get_latest_file_version(integer, integer);

CREATE OR REPLACE FUNCTION api.get_latest_file_version(
    session_key text,
    file_id integer)
    RETURNS SETOF dbfile AS
$BODY$
DECLARE
    current_user_id integer;
    is_user_an_admin boolean;
	is_checked_out boolean;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

    current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);		

	--
    --
	SELECT true INTO is_checked_out FROM CheckOut WHERE CheckOut.FileID = file_id;

	IF (is_checked_out = TRUE AND ((SELECT CO.UserID FROM CheckOut CO WHERE CO.FileID = file_id) = current_user_id OR is_user_an_admin = TRUE)) THEN
		RETURN QUERY
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
			FROM
				File F, FileInstance FI, Checkout CO
			WHERE
				F.FileID = file_id AND
				FI.FileInstanceID = F.CheckedOutInstanceID AND
				CO.FileID = file_id AND
				(current_user_id = CO.UserID OR is_user_an_admin = TRUE);
	ELSE
		RETURN QUERY
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
			FROM
				File F, FileInstance FI, Changeset CS
			WHERE
				F.FileID = file_id AND
				FI.FileInstanceID = F.CheckedInInstanceID AND
				CS.ChangesetID = FI.ChangesetID;
	END IF;
END
$BODY$
LANGUAGE plpgsql;



DROP FUNCTION public.add_or_update_file(integer, text, text, text, bytea, text);

CREATE OR REPLACE FUNCTION api.add_or_update_file(
    session_key text,
    full_parent_file_name text,
    file_name text,
    checkin_comment text,
    file_data bytea,
    details text)
  RETURNS integer AS
$BODY$
DECLARE
	current_user_id integer;
    parent_file_id integer;
    file_id integer;
    file_state objectstate;
BEGIN
	-- EXAMPLE: SELECT * FROM add_or_update_file($(S_e_s_s_i_o_n_K_e_y), '$root$/MC/', 'ModulesConfigurations.descr', 'Check in cooment', 'file content, can be binary', '{}');
	--

    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	current_user_id := user_api.current_user_id(session_key);

    -- get parent file id, exception will occur if it dows not exists
    parent_file_id := get_file_id(current_user_id, full_parent_file_name);

    -- get file_id id it exists
    BEGIN
        file_id := get_file_id(current_user_id, parent_file_id, file_name);
    EXCEPTION WHEN OTHERS THEN
        -- File does not exists or it is in state of creation (added by other user but was not checked in yet)
    END;

    IF (file_id IS NULL) THEN
        -- try to add file (it will be checked out)
		file_id	:= (SELECT id FROM add_file(current_user_id, file_name, parent_file_id, file_data, details));
    ELSE
		-- check out file if it was not yet
		file_state := get_file_state(file_id);

		IF (file_state.checkedout = FALSE) THEN
			file_state := check_out(current_user_id, ARRAY[file_id]);

			IF (file_state.checkedout = FALSE) THEN
				RAISE EXCEPTION 'Check out error %', file_name;
			END IF;

		END IF;

		IF (file_state.deleted = TRUE) THEN
			RAISE EXCEPTION 'File %/% marked as deleted, cannot update file.', full_parent_file_name, file_name;
		END IF;

		-- set workcopy
		PERFORM api.set_workcopy(session_key, file_id, file_data, details);
    END IF;

    -- check in
    PERFORM check_in(current_user_id, ARRAY[file_id], checkin_comment);

    RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;

