CREATE OR REPLACE FUNCTION get_checked_out_signals_ids(
    user_id integer)
  RETURNS SETOF integer AS
$BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	RETURN QUERY
		SELECT SignalID 
			FROM CheckOut 
			WHERE SignalID IS NOT NULL AND (UserID = user_id OR userIsAdmin)
			ORDER BY SignalID ASC;
END
$BODY$
LANGUAGE plpgsql;
