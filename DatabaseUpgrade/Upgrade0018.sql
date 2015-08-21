CREATE OR REPLACE FUNCTION checkin(file_ids integer[], user_id integer, checkin_comment text)
RETURNS integer AS
$BODY$
DECLARE
	NewChangesetID int;
	WasCheckedOut int;
	file_id int;
BEGIN
	-- Check if files really checked out
	FOREACH file_id IN ARRAY file_ids
	LOOP
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			RAISE 'File is not checked out: %', file_id;
			RETURN FALSE;
		END IF;
	END LOOP;

	--
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, TRUE) RETURNING ChangesetID INTO NewChangesetID;

	UPDATE FileInstance SET ChangesetID = NewChangesetID WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);

	RETURN NewChangesetID;
END;
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION checkout(file_ids integer[], user_id integer)
RETURNS boolean AS
$BODY$
DECLARE
	AlreadyCheckedOut integer;
	file_id integer;
BEGIN
	-- Check if file is not checked out already
	SELECT count(CheckOutID) INTO AlreadyCheckedOut FROM CheckOut WHERE FileID = ANY(file_ids);
	IF (AlreadyCheckedOut > 0) THEN
		RAISE 'Files already checked out';
		RETURN FALSE;
	END IF;

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Add record to the CheckOutTable
		INSERT INTO CheckOut (UserID, FileID) VALUES (user_id, file_id);

		-- Make new work copy in FileInstance
		INSERT INTO
			FileInstance (Data, Size, FileID, ChangesetID, Action)
			SELECT
				Data, length(Data) AS Size, FileId, NULL, 2
			FROM
				FileInstance
			WHERE
				FileID = file_id AND
				Sequence = (SELECT MAX(Sequence) FROM FileInstance WHERE FileID = file_id);

	END LOOP;

	RETURN TRUE;
END
$BODY$
LANGUAGE plpgsql;

-- Delete the old function
DROP FUNCTION undofilependingchanges(integer, integer);

CREATE OR REPLACE FUNCTION undochanges(file_ids integer[], user_id integer)
RETURNS integer AS
$BODY$
DECLARE
	WasCheckedOut int;
	file_id int;
	file_user int;
	is_user_admin boolean;
BEGIN
	SELECT Users.Administrator INTO is_user_admin FROM Users WHERE Users.UserID = user_id;

	FOREACH file_id IN ARRAY file_ids
	LOOP
		-- Check if files really checked out
		SELECT count(*) INTO WasCheckedOut FROM CheckOut WHERE FileID = file_id;
		IF (WasCheckedOut <> 1)	THEN
			RAISE 'File is not checked out: %', file_id;
			RETURN FALSE;
		END IF;

		-- Check if the file can be undo by this user_id
		SELECT UserId INTO file_user FROM CheckOut WHERE FileID = file_id;

		IF (is_user_admin = FALSE AND file_user <> user_id) THEN
			RAISE 'User % has no right to perform operation.', user_id;
			RETURN FALSE;
		END IF;

	END LOOP;

	DELETE FROM FileInstance WHERE FileID = ANY(file_ids) AND ChangesetID IS NULL;

	DELETE FROM CheckOut WHERE FileID = ANY(file_ids);

	FOREACH file_id IN ARRAY file_ids
	LOOP
		DELETE FROM File
			WHERE FileID = file_id AND NOT EXISTS (SELECT * FROM FileInstance WHERE FileId = file_id);
	END LOOP;

	RETURN 0;
END;
$BODY$
LANGUAGE plpgsql;
