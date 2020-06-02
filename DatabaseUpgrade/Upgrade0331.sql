CREATE OR REPLACE FUNCTION public.set_signal_workcopy(
    user_id integer,
	sd signaldata)
  RETURNS objectstate AS
$BODY$
DECLARE
    chOutInstanceID integer;
	chOutUserID integer;
	os objectstate;
	signalID integer;

    userIsAdmin boolean = is_admin(user_id);

    findAppSignalID integer [];

    findCustomAppSignalID integer [];
BEGIN
    SELECT
	    S.CheckedOutInstanceID, S.UserID, S.SignalID INTO chOutInstanceID, chOutUserID, signalID
	FROM
	    Signal AS S
	WHERE
	    S.SignalID = sd.SignalID;

    findAppSignalID = ARRAY(SELECT * FROM get_signal_ids_with_appsignalid(user_id, sd.AppSignalID));

    IF (position('$(' in sd.CustomAppSignalID) != 0) THEN
	    findCustomAppSignalID = ARRAY[signalID];	-- exclude uniquiness checking for customAppSignalID with macrosses like $(...)
	ELSE
	    findCustomAppSignalID = ARRAY(SELECT * FROM get_signal_ids_with_customappsignalid(user_id, sd.CustomAppSignalID));
	END IF;

    IF (chOutInstanceID IS NOT NULL) AND
	    (chOutUserID IS NOT NULL) AND
		(chOutUserID = user_id OR userIsAdmin)
	THEN
	    IF (array_length(findAppSignalID, 1) > 1) OR (array_length(findAppSignalID, 1) = 1 AND findAppSignalID[1] != signalID) THEN
		    RAISE USING ERRCODE = '55011';
		END IF;

        IF (array_length(findCustomAppSignalID, 1) > 1) OR (array_length(findCustomAppSignalID, 1) = 1 AND findCustomAppSignalID[1] != signalID) THEN
		    RAISE USING ERRCODE = '55022';
		END IF;

        -- update checked out workcopy
		UPDATE SignalInstance SET
		    AppSignalID = sd.AppSignalID,
			CustomAppSignalID = sd.CustomAppSignalID,
			EquipmentID = sd.EquipmentID,
			-- Channel is not updatable

            -- SignalType is not updatable
			InOutType = sd.InOutType,

            SpecPropStruct = sd.SpecPropStruct,
			SpecPropValues = sd.SpecPropValues,
			ProtoData = sd.ProtoData

            -- other fields from SignalData is not updatable
		WHERE
		    SignalInstanceID = chOutInstanceID;

        os.ID = sd.SignalID;
		os.deleted = FALSE;
		os.checkedout = TRUE;
		os.action = 0;
		os.userID = chOutUserID;
		os.errCode = 0;					-- ERR_SIGNAL_OK
	ELSE
	    IF chOutInstanceID IS NULL THEN
		    os.ID = sd.SignalID;
			os.deleted = FALSE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 1;					-- ERR_SIGNAL_IS_NOT_CHECKED_OUT
		ELSE
		    os.ID = sd.SignalID;
			os.deleted = FALSE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = chOutUserID;
			os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT by another user
		END IF;
	END IF;

    RETURN os;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
