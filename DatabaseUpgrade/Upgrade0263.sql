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


CREATE OR REPLACE FUNCTION undo_signals_changes(
	user_id integer,
	signal_ids integer[])
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	os objectstate;
	userIsAdmin boolean;
	chInInstanceID integer;
	chOutInstanceID integer;
	chOutUserID integer;
	sGroupID integer;
	signal_id integer;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID, S.SignalGroupID
		INTO chInInstanceID, chOutInstanceID, chOutUserID, sGroupID
		FROM Signal AS S WHERE SignalID = signal_id;

		IF chOutInstanceID IS NULL THEN
			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 1;					-- ERR_SIGNAL_IS_NOT_CHECKED_OUT
			RETURN NEXT os;
			CONTINUE;
		END IF;

		IF chOutUserID = user_id OR userIsAdmin THEN
			DELETE FROM CheckOut WHERE SignalID = signal_id;
			UPDATE Signal SET CheckedOutInstanceID = NULL, UserID = NULL WHERE SignalID = signal_id;
			DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;

			os.deleted = FALSE;

			IF chInInstanceID IS NULL THEN
				DELETE FROM Signal WHERE SignalID = signal_id;

				os.deleted = TRUE;

				IF sGroupID <> 0 THEN
					DELETE FROM SignalGroup
					WHERE NOT EXISTS(SELECT * FROM Signal AS S WHERE SignalGroupID = sGroupID) AND SignalGroupID = sGroupID;
				END IF;
			END IF;

			os.ID = signal_id;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 0;					-- ERR_SIGNAL_OK
			RETURN NEXT os;

		ELSE
			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = TRUE;
			os.action = 0;
			os.userID = chOutUserID;
			os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
			RETURN NEXT os;
		END IF;
	END LOOP;
END
$BODY$
  LANGUAGE plpgsql;

