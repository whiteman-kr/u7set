CREATE OR REPLACE FUNCTION public.delete_signal(
	user_id integer,
	signal_id integer)
	RETURNS objectstate
	LANGUAGE 'plpgsql'

	COST 100
	VOLATILE
AS $BODY$
DECLARE
	os objectstate;
	chInInstanceID integer;
	chOutInstanceID integer;
	chOutUserID integer;
	sGroupID integer;
BEGIN
	SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID, S.SignalGroupID
	INTO chInInstanceID, chOutInstanceID, chOutUserID, sGroupID
	FROM Signal AS S WHERE SignalID = signal_id;

	IF chInInstanceID IS NULL THEN
		IF user_id <> chOutUserID THEN
			-- signal is checked out by another user

			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = TRUE;
			os.action = 0;
			os.userID = chOutUserID;
			os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT

			RETURN os;
		END IF;

		-- no checked in instance
		-- delete checked out instance and signal
		DELETE FROM CheckOut WHERE SignalID = signal_id;
		UPDATE Signal SET CheckedOutInstanceID = NULL WHERE SignalID = signal_id;
		DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;
		DELETE FROM Signal WHERE SignalID = signal_id;

		IF sGroupID <> 0 THEN
			DELETE FROM SignalGroup
			WHERE NOT EXISTS(SELECT * FROM Signal AS S WHERE SignalGroupID = sGroupID) AND SignalGroupID = sGroupID;
		END IF;

		os.ID = signal_id;
		os.deleted = TRUE;
		os.checkedout = FALSE;
		os.action = 3;
		os.userID = 0;
		os.errCode = 0;					-- ERR_SIGNAL_OK
		RETURN os;
	ELSE
		-- has checked in instance

		IF chOutInstanceID IS NULL THEN
			-- signal is not checked out, - check out signal
			os = checkout_signals(user_id, ARRAY[signal_id]);
			IF os.errCode <> 0 THEN
				RETURN os;
			END IF;

			SELECT CheckedOutInstanceID, UserID INTO chOutInstanceID, chOutUserID FROM Signal WHERE SignalID = signal_id;
		END IF;

		-- signal is checked out

		IF chOutUserID = user_id THEN
			-- signal checked out by this user
			-- change checked out signal instance action to Delete (3)

			UPDATE SignalInstance
			SET Action = 3
			WHERE SignalInstanceID = (SELECT CheckedOutInstanceID FROM Signal WHERE SignalID = signal_id);

			os.ID = signal_id;
			os.deleted = TRUE;
			os.checkedout = TRUE;
			os.action = 3;
			os.userID = user_id;
			os.errCode = 0;					-- ERR_SIGNAL_OK
		ELSE
			-- signal is checked out by another user

			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = TRUE;
			os.action = 0;
			os.userID = chOutUserID;
			os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
		END IF;

		RETURN os;
	END IF;
END
$BODY$;


CREATE OR REPLACE FUNCTION get_signals_actual_signalinstanceid(user_id INTEGER, with_deleted BOOLEAN) RETURNS SETOF integer
LANGUAGE PLPGSQL
AS $plpgsql$
DECLARE
  userIsAdmin BOOLEAN;
BEGIN
  SELECT
	is_admin(user_id) INTO userIsAdmin;

  RETURN QUERY

		  SELECT
			SI.SignalInstanceID
		  FROM Signal AS S,
			   SignalInstance AS SI,
			   Changeset AS CS
		  WHERE SI.SignalID = S.SignalID
			AND (S.Deleted = FALSE OR with_deleted = TRUE)
			AND SI.ChangesetID = CS.ChangesetID
			AND SI.SignalInstanceID IN (SELECT
			  SG.CheckedInInstanceID
			FROM Signal AS SG
			WHERE (SG.CheckedOutInstanceID IS NULL
			OR (SG.UserID <> user_id
			AND userIsAdmin = FALSE)))

		  UNION ALL

		  SELECT
			SI.SignalInstanceID
		  FROM Signal AS S,
			   SignalInstance AS SI
		  WHERE SI.SignalID = S.SignalID
		  AND (S.Deleted = FALSE OR with_deleted = TRUE)
		  AND SI.SignalInstanceID IN (SELECT
			  SG.CheckedOutInstanceID
			FROM Signal AS SG
			WHERE (SG.CheckedOutInstanceID IS NOT NULL
			AND (SG.UserID = user_id
			OR userIsAdmin = TRUE)));
END
$plpgsql$;


CREATE OR REPLACE FUNCTION public.get_signals_id_appsignalid(
	user_id integer,
	with_deleted boolean)
	RETURNS SETOF signal_id_appsignalid
	LANGUAGE 'plpgsql'

	COST 100
	VOLATILE
	ROWS 1000
AS $BODY$
BEGIN
	RETURN QUERY
		SELECT SI.SignalID, SI.AppSignalID
		FROM
			SignalInstance AS SI
		WHERE
			SI.SignalInstanceID IN
			(SELECT * FROM get_signals_actual_signalinstanceid(user_id, with_deleted))
		ORDER BY
			SI.SignalID ASC;
END
$BODY$;


CREATE OR REPLACE FUNCTION get_signals_unique_specpropstructs (user_id INTEGER, with_deleted BOOLEAN) RETURNS SETOF text
LANGUAGE PLPGSQL
AS $plpgsql$
BEGIN
  RETURN QUERY
	  SELECT DISTINCT TRIM(BOTH FROM SINST.specpropstruct) FROM signalinstance AS SINST
			WHERE (SINST.specpropstruct IS NOT NULL) AND SINST.specpropstruct!='' AND
				SINST.signalinstanceid IN
					(SELECT * FROM get_signals_actual_signalinstanceid(user_id, with_deleted));
END
$plpgsql$;