CREATE OR REPLACE FUNCTION api.get_checked_out_files(
    session_key text,
    parent_file_ids integer[])
  RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
	user_id integer;
	parent_id integer;
	checked_out_ids integer[];
BEGIN
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);
	
	user_id := user_api.current_user_id(session_key);

	-- Get files list
	--
	FOREACH parent_id IN ARRAY parent_file_ids
	LOOP
		-- if parent id is 0 (root) request checked out files from table CheckOut
		IF (parent_id = 0)	
		THEN
			IF (user_api.is_current_user_admin(session_key) = TRUE)
			THEN
				-- if user is admin, request all files from table CheckOut
				checked_out_ids := array_cat(ARRAY(SELECT FileID FROM CheckOut WHERE FileID IS NOT NULL), checked_out_ids);
			ELSE
				-- if user is not admin, request only user's files from table CheckOut
				checked_out_ids := array_cat(ARRAY(SELECT FileID FROM CheckOut WHERE FileID IS NOT NULL AND UserId = user_id), checked_out_ids);
			END IF;		
		ELSE
			-- parent_id is not null, so get file list by parent
			checked_out_ids := array_cat(ARRAY(SELECT FileID FROM api.get_file_list_tree(session_key, parent_id, '%', true) WHERE CheckedOut = TRUE), checked_out_ids);
		END IF;
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
