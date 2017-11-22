DROP FUNCTION public.get_file_info(integer, integer[]);

CREATE OR REPLACE FUNCTION api.get_file_info(
    session_key text,
    file_ids integer[])
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

    RETURN QUERY
    (
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            F.ChangesetID AS ChangesetID,
            F.Created AS Created,
            F.Size AS Size,
            F.ChangesetID IS NULL AS CheckedOut,
            Changeset.time AS CheckOutTime,
            Changeset.UserID AS UserID,
            F.Action AS Action,
            F.Details AS Details
        FROM
            -- All checked in now
            (SELECT
                F.FileID AS FileID,
                F.Deleted AS Deleted,
                F.Name AS Name,
                F.ParentID AS ParentID,
                F.Created AS Created,
                FI.FileInstanceID AS FileInstanceID,
                FI.ChangesetID AS ChangesetID,
                length(FI.data) AS Size,
                FI.Created AS InstanceCreated,
                FI.Action AS Action,
                FI.Details::text AS Details
            FROM
                File F,
                FileInstance FI
            WHERE
                F.FileID = ANY(file_ids) AND
                F.CheckedInInstanceID = FI.FileInstanceID AND
                F.CheckedOutInstanceID IS NULL AND
                F.FileID = FI.FileID
            ) AS F
            LEFT JOIN
            Changeset USING (ChangesetID))
        UNION
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            F.ChangesetID AS ChangesetID,
            F.Created AS Created,
            F.Size AS Size,
            F.ChangesetID IS NULL AS CheckedOut,
            CheckOut.time AS CheckOutTime,
            CheckOut.UserID AS UserID,
            F.Action AS Action,
            F.Details AS Details
        FROM
            -- All CheckedOut by any user if user_id is administrator
            (SELECT
                F.FileID AS FileID,
                F.Deleted AS Deleted,
                F.Name AS Name,
                F.ParentID AS ParentID,
                F.Created AS Created,
                FI.FileInstanceID AS FileInstanceID,
                FI.ChangesetID AS ChangesetID,
                length(FI.data) AS Size,
                FI.Created AS InstanceCreated,
                FI.Action AS Action,
                FI.Details::text AS Details
            FROM
                File F,
                FileInstance FI,
                CheckOut CO
            WHERE
                F.FileID = ANY(file_ids) AND
                F.CheckedOutInstanceID = FI.FileInstanceID AND
                F.FileID = FI.FileID AND
                F.FileID = CO.FileID AND
                (is_user_an_admin = TRUE OR CO.UserID = current_user_id)
            ) AS F
            LEFT JOIN
            CheckOut USING (FileID))
        ORDER BY Name
    );

END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.is_file_exists(session_key text, parent_id integer, file_name text)
    RETURNS integer AS
$BODY$
DECLARE
    current_user_id integer;
    result integer;
BEGIN
    -- Function returns FileID if file exists for the current user or NULL if not
    --

    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	IF (user_api.is_current_user_admin(session_key) = TRUE)
	THEN
        -- Administrator can see all objects, even just created and not checked in yet
        --
        SELECT F.FileID INTO result
            FROM 
                File F
            WHERE 
                F.ParentID = parent_id AND
                F.Name = file_name AND
                F.Deleted = FALSE;
	ELSE
        -- Not admin can see only checked in versions or own files
        --
        current_user_id := user_api.current_user_id(session_key);
        
        SELECT F.FileID INTO result
            FROM 
                File F
            WHERE 
                F.ParentID = parent_id AND
                F.Name = file_name AND
                F.Deleted = FALSE AND
                (F.CheckedInInstanceID IS NOT NULL OR           -- If F.CheckedInInstanceID is not null, then it definately just created and checked out, check who's done it
                    current_user_id = (SELECT UserID FROM CheckOut WHERE CheckOut.FileID = F.FileID));
    END IF;

    RETURN result;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_file_info(session_key text, parent_id integer, file_name text)
    RETURNS dbfileinfo AS
$BODY$
DECLARE
    file_id integer;
    result dbfileinfo;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    file_id := (SELECT * FROM api.is_file_exists(session_key, parent_id, file_name));

    IF (file_id IS NULL)
    THEN
        RAISE 'File % does not exist.', file_name;
    END IF;

    SELECT * FROM api.get_file_info(session_key, file_id)
        INTO result;

    RETURN result;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_file_info(session_key text, file_id integer)
    RETURNS dbfileinfo AS
$BODY$
DECLARE
    current_user_id integer;
    is_user_an_admin boolean;
    result dbfileinfo;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);

	SELECT
        F.FileID AS FileID,
        F.Deleted AS Deleted,
        F.Name AS Name,
        F.ParentID AS ParentID,
        F.ChangesetID AS ChangesetID,
        F.Created AS Created,
        F.Size AS Size,
        F.ChangesetID IS NULL AS CheckedOut,
        Changeset.time AS CheckOutTime,
        Changeset.UserID AS UserID,
        F.Action AS Action,
        F.Details AS Details
    FROM
        -- If file All checked in now
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            F.Created AS Created,
            FI.FileInstanceID AS FileInstanceID,
            FI.ChangesetID AS ChangesetID,
            length(FI.data) AS Size,
            FI.Created AS InstanceCreated,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI
        WHERE
            F.FileID = file_id AND
            F.CheckedInInstanceID = FI.FileInstanceID AND
            F.CheckedOutInstanceID IS NULL AND
            F.FileID = FI.FileID
        ) AS F
    LEFT JOIN
    Changeset USING (ChangesetID)
    UNION
    SELECT
        F.FileID AS FileID,
        F.Deleted AS Deleted,
        F.Name AS Name,
        F.ParentID AS ParentID,
        F.ChangesetID AS ChangesetID,
        F.Created AS Created,
        F.Size AS Size,
        F.ChangesetID IS NULL AS CheckedOut,
        CheckOut.time AS CheckOutTime,
        CheckOut.UserID AS UserID,
        F.Action AS Action,
        F.Details AS Details
    FROM
        -- All CheckedOut by any user if current_user_id is administrator
        (SELECT
            F.FileID AS FileID,
            F.Deleted AS Deleted,
            F.Name AS Name,
            F.ParentID AS ParentID,
            F.Created AS Created,
            FI.FileInstanceID AS FileInstanceID,
            FI.ChangesetID AS ChangesetID,
            length(FI.data) AS Size,
            FI.Created AS InstanceCreated,
            FI.Action AS Action,
            FI.Details::text AS Details
        FROM
            File F,
            FileInstance FI,
            CheckOut CO
        WHERE
            F.FileID = file_id AND
            F.CheckedOutInstanceID = FI.FileInstanceID AND
            F.FileID = FI.FileID AND
            F.FileID = CO.FileID AND
            (CO.UserID = current_user_id OR is_user_an_admin = TRUE)
        ) AS F
        LEFT JOIN
        CheckOut USING (FileID) 
        INTO result;    -- INTO result !!!

    IF (result IS NULL)
    THEN
        RAISE 'File % does not exist.', file_id;
    END IF;

    RETURN result;
END
$BODY$
LANGUAGE plpgsql;

-- Create safe veriosn of get_workcopy
--
DROP FUNCTION public.get_workcopy(integer, integer);

CREATE OR REPLACE FUNCTION api.get_workcopy(session_key text, file_id integer)
  RETURNS dbfile AS
$BODY$
DECLARE
	checked_out_instance_id uuid;
	checked_out_by_user integer;
	current_user_id integer;
	is_user_an_admin boolean;
	result DbFile;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	-- Check if the file checked out
	--
	checked_out_instance_id := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);
	IF (checked_out_instance_id IS NULL)
	THEN
		RAISE EXCEPTION 'File % is not checked out or does not exist', file_id;
	END IF;

	-- check if a file is checked out by the same user or user is an administartor
	--
	current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);	
	checked_out_by_user := (SELECT UserID FROM CheckOut WHERE FileID = file_id);
	
	IF (checked_out_by_user IS NULL) OR
		(is_user_an_admin = FALSE AND current_user_id != checked_out_by_user)
	THEN
		RAISE 'File % is checked out by user %', file_id, (SELECT username FROM Users WHERE UserID = checked_out_by_user);
	END IF;

	-- Select data into result variable
	--
	SELECT F.FileID AS FileID,
		F.Deleted AS Deleted,
		F.Name AS Name,
		F.ParentID AS ParentID,
		0 AS ChangesetID,
		F.Created AS Created,
		length(FI.Data) AS Size,
		FI.Data AS Data,
		TRUE AS CheckedOut,
		CO.Time As ChechoutTime,
		CO.UserID AS UserID,
		FI.Action AS Action,
		FI.Details::text AS Details
	INTO
		result
	FROM
		File F, FileInstance FI, Checkout CO
	WHERE
		F.FileID = file_id AND
		FI.FileInstanceID = F.CheckedOutInstanceID AND
		CO.FileID = file_id AND
		(current_user_id = CO.UserID OR is_user_an_admin = TRUE);

	RETURN result;
END;
$BODY$
LANGUAGE plpgsql;


-- Create safe veriosn of  set_workcopy
--
DROP FUNCTION public.set_workcopy(integer, integer, bytea, text);

CREATE OR REPLACE FUNCTION api.set_workcopy(
    session_key text,
    file_id integer,
    file_data bytea,
    in_details text)
  RETURNS integer AS
$BODY$
DECLARE
    user_allowed integer;
	inst_file_id integer;
	fileinstance_uuid uuid;
	current_user_id integer;
	is_user_an_admin boolean;	
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    current_user_id := user_api.current_user_id(session_key);
	is_user_an_admin := user_api.is_current_user_admin(session_key);	

    SELECT count(*) INTO user_allowed
	    FROM CheckOut WHERE FileID = file_id AND UserID = current_user_id;

    IF (user_allowed = 0 AND is_user_an_admin = FALSE) THEN
	    RAISE 'User is not allowed to set workcopy for file_id %', file_id;
	END IF;

    fileinstance_uuid := (SELECT CheckedOutInstanceID FROM File WHERE FileID = file_id);

    IF (fileinstance_uuid IS NULL) THEN
	    RAISE 'File % is not checked out', file_id;
	END IF;

    UPDATE FileInstance SET Size = length(file_data), Data = file_data, Details = in_details::JSONB, md5 = md5(file_data)
	    WHERE FileInstanceID = fileinstance_uuid
		RETURNING FileID INTO inst_file_id;

    IF (inst_file_id <> file_id) THEN
	    RAISE 'DATABASE CRITICAL ERROR, FileID in File and FileInstance tables is different! %', file_id;
	END IF;

    RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;