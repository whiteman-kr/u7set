--
--  RPCT-1213 Remove Administartor right for all users except Administrator (UserID = 1)
--

-- Remove Administartor right for all users except Administrator (UserID = 1)
--
UPDATE Users SET Administrator = FALSE WHERE UserID <> 1;


-------------------------------------------------------------------------------
--
-- Remove ability to update Administartor column 
--
-------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION update_user(
	current_user_id integer,
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


