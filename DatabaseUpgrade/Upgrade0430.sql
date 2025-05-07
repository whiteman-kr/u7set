-- FUNCTION: api.get_file_info(text, integer)
--
CREATE OR REPLACE FUNCTION api.get_file_info(
    session_key text,
    file_id integer)
    RETURNS dbfileinfo
    LANGUAGE 'plpgsql'
AS $BODY$
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
        -- File CheckedOut by other user and current user is not admin.
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
            FI.FileInstanceID = F.CheckedInInstanceID AND
            CO.FileID = F.FileID AND
            (CO.UserID <> current_user_id AND is_user_an_admin = FALSE)
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
$BODY$;


-- FUNCTION: api.get_file_info(text, integer[])
--
CREATE OR REPLACE FUNCTION api.get_file_info(
    session_key text,
    file_ids integer[])
    RETURNS SETOF dbfileinfo 
    LANGUAGE 'plpgsql'
AS $BODY$
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
        -- File is CheckedOut by other user and current user is not an admin.
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
            FI.FileInstanceID = F.CheckedInInstanceID AND
            CO.FileID = F.FileID AND
            (CO.UserID <> current_user_id AND is_user_an_admin = FALSE)
       ) AS F
       LEFT JOIN
       CheckOut USING (FileID))
        ORDER BY Name
    );

END
$BODY$;
