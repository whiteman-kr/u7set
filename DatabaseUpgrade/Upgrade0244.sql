ALTER TYPE public.dbfileinfo
    ADD ATTRIBUTE attributes integer;

ALTER TYPE public.dbfile
    ADD ATTRIBUTE attributes integer; 

ALTER TABLE public.file
   ADD COLUMN attributes integer NOT NULL DEFAULT 0;


CREATE OR REPLACE FUNCTION api.set_file_attributes(
    session_key text,
    full_file_name text,
    attr integer)
  RETURNS dbfileinfo AS
$BODY$
DECLARE
    file_id integer;
	return_value dbfileinfo;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Get file id by full path and name, exception occurs if file not found
    --
	file_id := api.get_file_id(session_key, full_file_name);

    -- set attributes
    --
    UPDATE File SET Attributes = attr WHERE FileID = file_id;

    return_value := api.get_file_info(session_key, file_id);
	RETURN return_value;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.set_file_attributes(
    session_key text,
    file_id integer,
    attr integer)
  RETURNS dbfileinfo AS
$BODY$
DECLARE
	return_value dbfileinfo;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- set attributes
    --
    UPDATE File SET Attributes = attr WHERE FileID = file_id;

    return_value := api.get_file_info(session_key, file_id);
	RETURN return_value;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION api.get_file_info(
    session_key text,
    file_id integer)
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
        F.Details AS Details,
        F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
        F.Details AS Details,
        F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            F.Details AS Details,
            F.Attributes As Attributes
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
                FI.Details::text AS Details,
                F.Attributes As Attributes
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
            F.Details AS Details,
            F.Attributes As Attributes
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
                FI.Details::text AS Details,
                F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
            FI.Details::text AS Details,
            F.Attributes As Attributes
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
				FI.Details::text AS Details,
                F.Attributes As Attributes
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
				FI.Details::text AS Details,
                F.Attributes As Attributes
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
				FI.Details::text AS Details,
                F.Attributes As Attributes
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
				FI.Details::text AS Details,
                F.Attributes As Attributes
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


CREATE OR REPLACE FUNCTION api.get_workcopy(
    session_key text,
    file_id integer)
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
		FI.Details::text AS Details,
        F.Attributes As Attributes
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


-- api.add_file - Add attributes

DROP FUNCTION api.add_file(text, text, integer, bytea, text);

CREATE OR REPLACE FUNCTION api.add_file(
    session_key text,
    file_name text,
    parent_id integer,
    file_data bytea,
    details text,
    attributes integer)
  RETURNS objectstate AS
$BODY$
DECLARE
    exists int;
    user_id integer;
	newfileid int;
	newfileinstanceid uuid;
	return_value ObjectState;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);    

    SELECT count(*) INTO exists FROM File WHERE Name = file_name AND ParentID = parent_id AND Deleted = false;
	IF (exists > 0) THEN
	    RAISE 'File % already exists', file_name;
	END IF;

    INSERT INTO File (Name, ParentID, Deleted, Attributes)
	    VALUES (file_name, parent_id, false, attributes) RETURNING FileID INTO newfileid;

    INSERT INTO CheckOut (UserID, FileID)
	    VALUES (user_id, newfileid);

    INSERT INTO FileInstance (FileID, Size, Data, Action, Details, md5)
	    VALUES (newfileid, length(file_data), file_data, 1, details::jsonb, md5(file_data))
		RETURNING FileInstanceID INTO newfileinstanceid;

    UPDATE File SET CheckedOutInstanceID = newfileinstanceid WHERE FileID = newfileid;

    return_value := ROW(newfileid, FALSE, TRUE, 1, user_id, 0);
	RETURN return_value;
END
$BODY$
  LANGUAGE plpgsql;

-- add_unique_file - Add attributes

DROP FUNCTION api.add_unique_file(text, text, integer, integer, bytea, text);

CREATE OR REPLACE FUNCTION api.add_unique_file(
    session_key text,
    file_name text,
    parent_id integer,
    unique_from_file_id integer,
    file_data bytea,
    details text,
    attributes integer)
  RETURNS objectstate AS
$BODY$
DECLARE
	user_id integer;
    file_id integer;
    file_exists int;
	file_pattern text;
	return_value ObjectState;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

	user_id := user_api.current_user_id(session_key);

	-- Cut of extension as compariosion is done without it
	--
	file_pattern := left(file_name, char_length(file_name) - position('.' in reverse(file_name)));

    SELECT count(*) 
        INTO 
            file_exists 
        FROM 
            api.get_file_list_tree(session_key, unique_from_file_id, '%', true) AS F
        WHERE
            file_pattern ILIKE left(F.Name, char_length(F.Name) - position('.' in reverse(F.Name)));

	IF (file_exists > 0) THEN
	    RAISE 'File % is not unique, take into account that files are compared without extensions and compariosion is case insensetive.', file_pattern;
	END IF;	

    -- try to add file (it will be checked out)
    --
    return_value := api.add_file(session_key, file_name, parent_id, file_data, details, attributes);

    RETURN return_value;
END
$BODY$
  LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION public.get_file_list(
    user_id integer,
    parent_id integer)
  RETURNS SETOF dbfileinfo AS
$BODY$

	-- All checked in now
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
	FROM
		File F,
		FileInstance FI,
		CheckOut CO
	WHERE
		F.ParentID = parent_id AND
		FI.FileInstanceID = F.CheckedOutInstanceID AND
		--F.FileID = FI.FileID AND		-- it is done by (F.CheckedOutInstanceID = FI.FileInstanceID)
		CO.FileID = F.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE)
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
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
		(CO.UserID <> user_id AND (SELECT is_admin(user_id)) = FALSE)
	)

$BODY$
  LANGUAGE sql;


CREATE OR REPLACE FUNCTION public.get_file_list(
    user_id integer,
    parent_id integer,
    file_mask text)
  RETURNS SETOF dbfileinfo AS
$BODY$
	-- All checked in now
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
	FROM
		File F,
		FileInstance FI,
		CheckOut CO
	WHERE
		F.ParentID = parent_id AND
		FI.FileInstanceID = F.CheckedOutInstanceID AND
		--F.FileID = FI.FileID AND		-- it is done by (F.CheckedOutInstanceID = FI.FileInstanceID)
		CO.FileID = F.FileID AND
		(CO.UserID = user_id OR (SELECT is_admin(user_id)) = TRUE) AND
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
		FI.Details::text AS Details,
		F.Attributes AS Attributes
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
		(CO.UserID <> user_id AND (SELECT is_admin(user_id)) = FALSE) AND
		F.Name ILIKE file_mask
	)

$BODY$
  LANGUAGE sql;
