-- RPCT-652

-- Forbid to create usernames in different register (Designer and designer are the same users)
--
CREATE UNIQUE INDEX username_unique_idx on users (LOWER(username));

-------------------------------------------------------------------------------
--
--		get_user_id, allow to use case insensitive username
--
-- Upgrade 43, RPCT-652, username = user_name changed to ILIKE
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_user_id (IN user_name text, IN user_password text) RETURNS integer AS
$BODY$
	SELECT Users.UserID FROM Users
		WHERE 
			Users.Username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~' AND 
			Users.Password = user_password;
$BODY$
LANGUAGE sql;

-------------------------------------------------------------------------------
--
--		check_user_password
--
-- Upgrade 43, RPCT-652, username = user_name changed to ILIKE
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION check_user_password(IN user_name text, IN user_password text) RETURNS boolean AS
$BODY$
	SELECT COUNT(Users.UserID) > 0 FROM Users
		WHERE 
			Users.Username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~' AND 
			Users.Password = user_password;
$BODY$
LANGUAGE sql;

-------------------------------------------------------------------------------
--
--		create_user
--
-- Upgrade 43, RPCT-652, username = user_name changed to ILIKE
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION
	create_user(
		current_user_id integer,
		user_name text,
		first_name text,
		last_name text,
		user_password text,
		is_admin boolean,
		is_read_only boolean,
		is_disabled boolean)
RETURNS integer AS
$BODY$
DECLARE
	new_user_id integer;
BEGIN

	IF (is_admin(current_user_id) = FALSE OR is_admin(current_user_id) IS NULL) THEN
		RAISE 'User % has no right to add another user.', current_user_id;
	END IF;

	IF (SELECT count(*) FROM Users WHERE Username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~') > 0 THEN
		RAISE 'User % already exists.', user_name;
	END IF;

	IF (char_length(user_password) < 6) THEN
		RAISE 'Password is too simple, it must containe at least 6 symbols.';
	END IF;

	INSERT INTO Users (Username, FirstName, LastName, Password, Administrator, ReadOnly, Disabled)
		VALUES (user_name, first_name, last_name, user_password, is_admin, is_read_only, is_disabled) RETURNING UserID INTO new_user_id;

	RETURN new_user_id;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------------------------------------------------------
--
--		update_user
--
-- Upgrade 43, RPCT-652, username = user_name changed to ILIKE
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION update_user(
	current_user_id integer,
	user_name text,
	first_name text,
	last_name text,
	user_password text,
	new_password text,
	is_admin boolean,
	is_read_only boolean,
	is_disabled boolean)
	RETURNS integer AS
$BODY$
DECLARE
	user_id integer;
BEGIN
	user_id := (SELECT userid FROM users WHERE username ILIKE replace(replace(replace(user_name, '~', '~~'), '%', '~%'), '_', '~_') escape '~');

	IF (user_id IS NULL) THEN
		RAISE 'User % is not exists.', user_name;
	END IF;

	IF (user_id <> current_user_id AND
	   (is_admin(current_user_id) = FALSE OR is_admin(current_user_id) IS NULL)) THEN
		RAISE 'User % has no right to update another user.', current_user_id;
	END IF;

	-- update password if required (new_password is present)
	IF (new_password IS NOT NULL AND char_length(new_password) <> 0)
	THEN
		IF (current_user_id = user_id) AND
		   (SELECT password FROM users WHERE userid = user_id) <> user_password
		THEN
			RAISE 'Old password is incorrect';
		END IF;

		IF (char_length(new_password) < 6)
		THEN
			RAISE 'New password is too simple, it must contain at least 6 symbols.';
		END IF;

		UPDATE users SET password = new_password WHERE userid = user_id;
	END IF;

	-- upadte all other fields
	IF (is_admin(current_user_id) = TRUE)
	THEN
		UPDATE Users
			SET
				FirstName = first_name,
				LastName = last_name,
				Administrator = is_admin,
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
