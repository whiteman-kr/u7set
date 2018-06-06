CREATE OR REPLACE FUNCTION hascheckedoutsignals()
  RETURNS boolean AS
$BODY$
DECLARE
    checkedOutSignalsCount integer;
BEGIN
    SELECT count(*) INTO checkedOutSignalsCount FROM Checkout WHERE SignalID IS NOT NULL;

    RETURN checkedOutSignalsCount <> 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;


CREATE OR REPLACE FUNCTION public.checkout_signals(
    user_id integer,
    signal_ids integer[])
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	alreadyCheckedOut integer;
	signal_id integer;
	chOutInstanceID integer;
	os objectstate;
	chOutUserID integer;
	signalDeleted boolean;
	sgID integer;
	userIsAdmin boolean = is_admin(user_id);
BEGIN
	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT SignalID, CheckedOutInstanceID, UserID, Deleted INTO sgID, chOutInstanceID, chOutUserID, signalDeleted FROM Signal WHERE SignalID = signal_id;

		IF sgID IS NULL THEN

			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 4;					-- ERR_SIGNAL_NOT_FOUND
			RETURN NEXT os;

			CONTINUE;
		END IF;

		IF signalDeleted THEN
			-- signal deleted, can't check out

			os.ID = signal_id;
			os.deleted = TRUE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 3;					-- ERR_SIGNAL_DELETED
			RETURN NEXT os;

		ELSE
			IF chOutInstanceID IS NOT NULL THEN
				-- signal already checked Out

				os.ID = signal_id;
				os.deleted = FALSE;
				os.checkedout = TRUE;
				os.action = 0;
				os.userID = chOutUserID;

				IF user_id = chOutUserID OR userIsAdmin = TRUE THEN
					os.errCode = 0;					-- ERR_SIGNAL_OK
				ELSE
					os.errCode = 2;					-- ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				END IF;
				RETURN NEXT os;

			ELSE
				-- add record to the CheckOut table
				INSERT INTO CheckOut (UserID, SignalID) VALUES (user_id, signal_id);

				-- make new signal workcopy in SignalInstance
				INSERT INTO
					SignalInstance (
						AppSignalID,
						CustomAppSignalID,
						EquipmentID,

						InOutType,

						SpecPropStruct,
						SpecPropValues,
						ProtoData,

						SignalID,
						Action)
					SELECT
						AppSignalID,
						CustomAppSignalID,
						EquipmentID,

						InOutType,

						SpecPropStruct,
						SpecPropValues,
						ProtoData,

						SI.SignalID,
						2			-- Action Edit
					FROM
						Signal AS S,
						SignalInstance AS SI
					WHERE
						S.SignalID = signal_id AND
						SI.SignalID = signal_id AND
						SI.SignalInstanceID = S.CheckedInInstanceID
					RETURNING SignalInstanceID INTO chOutInstanceID;

				UPDATE Signal
				SET
					CheckedOutInstanceID = chOutInstanceID,
					UserId = user_id
				WHERE
					SignalID = signal_id;

				os.ID = signal_id;
				os.deleted = FALSE;
				os.checkedout = TRUE;
				os.action = 2;
				os.userID = user_id;
				os.errCode = 0;					-- ERR_SIGNAL_OK
				RETURN NEXT os;

			END IF;
		END IF;
	END LOOP;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
