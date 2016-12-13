-- RPCT-RPCT-1132 

CREATE EXTENSION pgcrypto;

CREATE SCHEMA api;
CREATE SCHEMA user_api;

-- Create crypto functions
--

CREATE OR REPLACE FUNCTION user_api.sha512(bytea) returns text AS $$
    SELECT encode(digest($1, 'sha512'), 'hex')
$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION user_api.sha512(text) returns text AS $$
    SELECT encode(digest($1, 'sha512'), 'hex')
$$ LANGUAGE SQL STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION user_api.password_hash(salt text, password text) returns text AS $$
    SELECT user_api.sha512(salt || password);
$$ LANGUAGE SQL STRICT IMMUTABLE;

-- Table users modification
--
ALTER TABLE public.users
    ADD COLUMN salt text NOT NULL DEFAULT user_api.sha512(gen_salt('bf'));

ALTER TABLE public.users 
    ADD COLUMN passwordhash text;
    
UPDATE public.users SET passwordhash = user_api.password_hash(salt, password);

ALTER TABLE public.users
   ALTER COLUMN passwordhash SET NOT NULL;

ALTER TABLE public.users 
    DROP COLUMN password;


-------------------------------------------------------------------------------
--
--		RPCT-1320, Add column md5 to FileInstance table
--
-------------------------------------------------------------------------------
ALTER TABLE public.fileinstance
    ADD COLUMN md5 text;

UPDATE public.fileinstance SET md5 = md5(data);

ALTER TABLE public.fileinstance
   ALTER COLUMN md5 SET NOT NULL;

CREATE OR REPLACE FUNCTION set_workcopy(
    user_id integer,
	file_id integer,
	file_data bytea,
	in_details text)
  RETURNS integer AS
$BODY$
DECLARE
    user_allowed int;
	inst_file_id int;
	fileinstance_uuid uuid;
BEGIN
    SELECT count(*) INTO user_allowed
	    FROM CheckOut WHERE FileID = file_id AND UserID = user_id;

    IF (user_allowed = 0 AND is_admin(user_id) = FALSE) THEN
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


CREATE OR REPLACE FUNCTION check_out(
    user_id integer,
	file_ids integer[])
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
    AlreadyCheckedOut integer;
	file_id integer;
	checkout_instance_id uuid;
	file_result ObjectState;
BEGIN
    -- Check if file is not checked out by other user,
	-- if it's already checked out by user_id then it is ok
	--
	SELECT count(CheckOutID) INTO AlreadyCheckedOut
	    FROM CheckOut
		WHERE FileID = ANY(file_ids) AND UserID <> user_id;

    IF (AlreadyCheckedOut > 0) THEN
	    RAISE 'Files already checked out';
	END IF;

    FOREACH file_id IN ARRAY file_ids
	LOOP
	    -- Check out only if file_id has not been already checked out
		--
		IF (SELECT count(CheckOutID) FROM CheckOut WHERE FileID = file_id) = 0
		THEN
		    -- Add record to the CheckOutTable
			INSERT INTO CheckOut (UserID, FileID) VALUES (user_id, file_id);

            -- Make new work copy in FileInstance
			INSERT INTO
			    FileInstance (Data, Size, FileID, ChangesetID, Action, Details, md5)
				SELECT
				    Data, length(Data) AS Size, FileId, NULL, 2, Details, md5(Data)
				FROM
				    FileInstance
				WHERE
				    FileID = file_id AND
					FileInstanceID = (SELECT CheckedInInstanceID FROM File WHERE FileID = file_id)
				RETURNING FileInstanceID INTO checkout_instance_id;

            -- Set CheckedOutInstanceID for File table
			UPDATE File SET CheckedOutInstanceID = checkout_instance_id WHERE FileID = file_id;
		END IF;

    END LOOP;

    -- Return checked out files
	--
	FOREACH file_id IN ARRAY file_ids
	LOOP
	    file_result := get_file_state(file_id);
		RETURN NEXT file_result;
	END LOOP;

    RETURN;
END
$BODY$
  LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		user_api.log_in -- RPCT-1321
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION user_api.log_in(user_name text, user_password text) 
RETURNS text AS
$BODY$
DECLARE
    already_logged_in boolean;
    password_is_correct boolean;
    user_id integer;
    user_salt text;
    session_key text;
BEGIN
    already_logged_in := (SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'session_table'));

    IF (already_logged_in = TRUE) 
    THEN
        RAISE 'User % already logged in.', (SELECT username FROM users, session_table WHERE users.userid = session_table.userid);
    END IF;

    user_id :=  (SELECT userid 
                 FROM users
                 WHERE username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~');
        
	password_is_correct := (SELECT EXISTS(
                                SELECT 1 FROM Users
                                WHERE 
                                    users.userid = user_id AND 
                                    users.passwordhash = user_api.password_hash(users.salt, user_password)));

	user_salt := (SELECT users.salt FROM users WHERE users.userid = user_id);
    
    IF (user_id IS NULL OR
        password_is_correct = FALSE OR 
        password_is_correct IS NULL OR
        user_salt IS NULL)
    THEN
        RAISE 'User does not exist or the password is incorrect.';
    END IF;

    CREATE TEMP TABLE session_table
    (
        userid int NOT NULL,                -- Loged in user id
        session_key text NOT NULL           -- Some unique key, is used in all changing data operations
    );

    INSERT INTO session_table (userid, session_key) VALUES (user_id, gen_salt('bf'))
        RETURNING session_table.session_key INTO session_key;
			
	RETURN session_key;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		user_api.log_out -- RPCT-1324
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION user_api.log_out() 
RETURNS integer AS
$BODY$
DECLARE
    is_logged_in boolean;
    user_id integer;
BEGIN
    is_logged_in := (SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'session_table'));

    IF (is_logged_in = FALSE) 
    THEN
        RAISE 'User is not logged in.';
    END IF;
    
    user_id := (SELECT userid FROM session_table);
        
    DROP TABLE session_table;
			
	RETURN user_id;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		user_api.check_session_key -- RPCT-1323
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION user_api.check_session_key(session_key text, raise_error boolean)
RETURNS boolean AS
$BODY$
DECLARE
    is_logged_in boolean;
    current_session_key text;
BEGIN
    is_logged_in := (SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'session_table'));

    IF (is_logged_in = FALSE) 
    THEN
        RAISE 'User is not logged in.';
    END IF;

    current_session_key := (SELECT ST.session_key FROM session_table ST);

    IF (current_session_key IS NULL OR
        current_session_key <> session_key)
    THEN
        IF (raise_error = TRUE)
        THEN
            RAISE 'Access violation.';
        ELSE
            RETURN FALSE;
        END IF;
            
    ELSE
        RETURN TRUE;
    END IF;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		user_api.current_user_id -- RPCT-1327
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION user_api.current_user_id(session_key text) 
RETURNS integer AS
$BODY$
DECLARE
    is_logged_in boolean;
    user_id integer;
BEGIN
    -- Check session_key and raise error if it is wrong
    --
    PERFORM user_api.check_session_key(session_key, TRUE);

    is_logged_in := (SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'session_table'));

    IF (is_logged_in = FALSE) 
    THEN
        RAISE 'User is not logged in.';
    END IF;

	RETURN (SELECT session_table.userid FROM session_table);
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--		user_api.is_current_user_admin -- RPCT-1328
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION user_api.is_current_user_admin(session_key text) 
RETURNS boolean AS
$BODY$
DECLARE
    is_logged_in boolean;
    user_id integer;
    result boolean;
BEGIN
    -- Check session_key and raise error if it is wrong
    --
    PERFORM user_api.check_session_key(session_key, TRUE);

    is_logged_in := (SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'session_table'));
    IF (is_logged_in = FALSE) 
    THEN
        RAISE 'User is not logged in.';
    END IF;

    user_id := (SELECT session_table.userid FROM session_table);

	result := (SELECT EXISTS 
                    (SELECT 1 FROM Users 
                    WHERE 
                        UserID = user_id AND
                        Administrator = TRUE AND
                        Disabled = FALSE));

    RETURN result;    
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--          update_user -- RPCT-1132
--
-------------------------------------------------------------------------------
DROP FUNCTION public.update_user(integer, text, text, text, text, text, boolean, boolean);
DROP FUNCTION public.update_user(integer, text, text, text, text, text, boolean, boolean, boolean);

CREATE OR REPLACE FUNCTION user_api.update_user(
    session_key text,
	user_name text,
	first_name text,
	last_name text,
	user_password text,
	new_password text,
	is_read_only boolean,
	is_disabled boolean)
	RETURNS integer AS
$BODY$
DECLARE
    user_id integer;
	user_salt text;
	user_password_hash text;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    user_id := (SELECT userid FROM users WHERE username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~');

    IF (user_id IS NULL) THEN
	    RAISE 'User % does not exists.', user_name;
	END IF;

    IF (user_id <> user_api.current_user_id(session_key) AND
	   user_api.is_current_user_admin(session_key) = FALSE)
	THEN
	    RAISE 'User % has no right to update another user.', (SELECT username FROM users WHERE userid = user_api.current_user_id(session_key));
	END IF;

    -- update password if required (new_password is present)
	IF (new_password IS NOT NULL AND char_length(new_password) <> 0)
	THEN
	    user_salt := (SELECT salt FROM users WHERE userid = user_id);

        IF (user_api.current_user_id(session_key) = user_id) AND
		   (SELECT passwordhash FROM users WHERE userid = user_id) <> user_api.password_hash(user_salt, user_password)
		THEN
		    RAISE 'Old password is incorrect';
		END IF;

        IF (char_length(new_password) < 6)
		THEN
		    RAISE 'New password is too simple, it must contain at least 6 symbols.';
		END IF;

        user_password_hash := user_api.password_hash(user_salt, new_password);

        UPDATE users SET passwordhash = user_password_hash WHERE userid = user_id;
	END IF;

    -- upadte all other fields
	--
	IF (user_api.is_current_user_admin(session_key) = TRUE)
	THEN
	    UPDATE Users
		    SET
			    FirstName = first_name,
				LastName = last_name,
				-- Administrator = is_admin,  -- This column can't be changed anymore
				ReadOnly = is_read_only,
				Disabled = is_disabled
			WHERE
			    userid = user_id;
	ELSE
	    UPDATE Users
		    SET
			    FirstName = first_name,
				LastName = last_name,
				--Administrator = is_admin,	-- If user is not administrator it has no right to change this columns
				ReadOnly = is_read_only
				--Disabled = is_disabled	-- If user is not administrator it has no right to change this columns
			WHERE
			    userid = user_id;
	END IF;

    -- return
	RETURN user_id;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--          DbUser -- RPCT-1332
--
-------------------------------------------------------------------------------
CREATE TYPE user_api.dbuser AS
(
    userid integer,
	username text,
	firstname text,
	lastname text,
	admininstrator boolean,
	readonly boolean,
	disabled boolean
);


-------------------------------------------------------------------------------
--
--          get_user_data -- RPCT-1132
--
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION user_api.get_user_data(
    session_key text,
	user_id int)
	RETURNS user_api.dbuser AS
$BODY$
DECLARE
    result user_api.dbuser;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    SELECT u.userid, u.username, u.firstname, u.lastname, u.administrator, u.readonly, u.disabled
	    INTO result
		FROM users u
		WHERE u.userid = user_id;

    IF (result IS NULL)
	THEN
	    RAISE 'User with id % does not exists.', user_id;
		END IF;

    RETURN result;
END
$BODY$
LANGUAGE plpgsql;


-------------------------------------------------------------------------------
--
--          check_user_password
--
-- This function does not need session key, as it's used to delete project
-------------------------------------------------------------------------------
DROP FUNCTION public.check_user_password(text, text);
DROP FUNCTION public.check_user_password(integer, text);

CREATE OR REPLACE FUNCTION user_api.check_user_password(user_name text, user_password text)
RETURNS text AS
$BODY$
DECLARE
    user_id integer;
	user_salt text;
	password_is_correct boolean;
BEGIN
    user_id :=  (SELECT userid
	             FROM users
				 WHERE username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~');


    password_is_correct := (SELECT EXISTS(
	                            SELECT 1 FROM Users
								WHERE
								    users.userid = user_id AND
									users.passwordhash = user_api.password_hash(users.salt, user_password)));

    user_salt := (SELECT users.salt FROM users WHERE users.userid = user_id);

    IF (user_id IS NULL OR
	    password_is_correct = FALSE OR
		password_is_correct IS NULL OR
		user_salt IS NULL)
		THEN
		RETURN FALSE;
		END IF;

    RETURN TRUE;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------------------------------------------------------
--
--          create_user
--
-------------------------------------------------------------------------------
DROP FUNCTION public.create_user(integer, text, text, text, text, boolean, boolean, boolean);

CREATE OR REPLACE FUNCTION user_api.create_user(
    session_key text,
	user_name text,
	first_name text,
	last_name text,
	user_password text,
	is_read_only boolean,
	is_disabled boolean)
  RETURNS integer AS
$BODY$
DECLARE
    user_salt text;
	user_password_hash text;
	new_user_id integer;
BEGIN

    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Check access rights
	--
	IF (user_api.is_current_user_admin(session_key) = FALSE)
	THEN
	    RAISE 'User % has no right to add another user.', (SELECT username FROM users WHERE userid = user_api.current_user_id(session_key));
	END IF;

    IF (SELECT count(*) FROM Users WHERE Username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~') > 0
	THEN
	    RAISE 'User % already exists.', user_name;
	END IF;

    IF (char_length(user_password) < 6) THEN
	    RAISE 'Password is too simple, it must contain at least 6 symbols.';
	END IF;

    user_salt := user_api.sha512(gen_salt('bf'));
	user_password_hash := user_api.sha512(user_salt || user_password);

    INSERT INTO Users (Username, FirstName, LastName, Salt, PasswordHash, Administrator, ReadOnly, Disabled)
	    VALUES (user_name, first_name, last_name, user_salt, user_password_hash, false, is_read_only, is_disabled)
		RETURNING UserID INTO new_user_id;

    RETURN new_user_id;
END
$BODY$
  LANGUAGE plpgsql;

-------------------------------------------------------------------------------
--
--          get_user_id -- deprecated
--
-------------------------------------------------------------------------------
DROP FUNCTION public.get_user_id(text, text);
