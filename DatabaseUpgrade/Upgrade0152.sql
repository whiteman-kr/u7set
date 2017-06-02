--
-- RPCT-1634
--

CREATE OR REPLACE FUNCTION public.delete_file_on_update(
    user_id integer,
    full_file_name text,
    checkin_comment text)
  RETURNS integer AS
$BODY$
DECLARE
    file_id integer;
    file_state objectstate;
BEGIN
    -- Function delete_file_on_update: Performs check_out, delete, check_in. 
    -- This function is useful for upgrading project db;
    --
	-- EXAMPLE: SELECT * FROM public.delete_file_on_update(1, '$root$/MC/ModulesConfigurations.descr', 'Check in cooment');
	--

    -- get file_id ià it exists
    BEGIN
        file_id := get_file_id(user_id, full_file_name);
    EXCEPTION WHEN OTHERS THEN
        -- File does not exists or it is in state of creation (added by other user but was not checked in yet)
    END;

    IF (file_id IS NULL) THEN
        RETURN file_id;
    END IF;

	-- check out file if it was not yet
	file_state := get_file_state(file_id);

	IF (file_state.deleted = TRUE) THEN
        -- File already deleted
        RETURN file_id;
	END IF;	

	IF (file_state.checkedout = FALSE) 
	THEN
		file_state := check_out(user_id, ARRAY[file_id]);

        IF (file_state.checkedout = FALSE) THEN
            RAISE EXCEPTION 'Check out error %', file_name;
        END IF;
	END IF;

	-- delete file
	file_state := delete_file(user_id, file_id);  

    -- file could be deleted completly (if it was not checked in before at least one time), in this case no need to perform check_in
	IF (file_state.deleted = TRUE) THEN
        -- File already deleted
        RETURN file_id;
	END IF;		
	
    -- check in
    PERFORM check_in(user_id, ARRAY[file_id], checkin_comment);

    RETURN file_id;
END
$BODY$
LANGUAGE plpgsql;



