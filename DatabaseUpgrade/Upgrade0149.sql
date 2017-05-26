--
-- RPCT-205 
--

DROP FUNCTION public.is_any_checked_out();

CREATE OR REPLACE FUNCTION api.is_any_checked_out(session_key text)
  RETURNS integer AS
$BODY$
DECLARE
    signalCount integer;
    userIsAdmin boolean;
	result integer;
    curentUserId integer;
BEGIN
    -- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);

    -- Check access rights
	--
	IF (user_api.is_current_user_admin(session_key) = TRUE)
	THEN
        -- Administrtor can access all objects, even checked out by other users
        --
        SELECT count(*) INTO result FROM CheckOut;
	ELSE
        -- Non administrator can view only own checked out files
        --
        curentUserId := user_api.current_user_id(session_key);
        SELECT count(*) INTO result FROM CheckOut WHERE UserID = curentUserId;
	END IF;
    
    RETURN result; 
END;
$BODY$
LANGUAGE plpgsql;



