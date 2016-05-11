-- RPCT-827
-- Field Channel beginning from 0


CREATE OR REPLACE FUNCTION public.add_signal(
    user_id integer,
    signal_type integer,
    channel_count integer)
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	newGroupID integer;
	channel integer;
	newSignalID integer;
	newSignalInstanceID integer;
	appSignalID varchar;
	customAppSignalID varchar;
	dataSize integer;
	os objectstate;
BEGIN
	IF channel_count < 1 THEN
		channel_count = 1;
	END IF;

	IF channel_count > 4 THEN
		RAISE 'Signal channelCount must be equal or less then 4';
	END IF;

	IF channel_count > 1 THEN
		INSERT INTO SignalGroup DEFAULT VALUES RETURNING SignalGroupID INTO newGroupID;
	ELSE
		-- all single-channel signals are placed in group 0
		newGroupID = 0;
	END IF;

	FOR channel IN 1..channel_count LOOP
		INSERT INTO Signal (SignalGroupID, Channel, Type, Deleted, UserID) VALUES (newGroupID, channel-1, signal_type, false, user_id) RETURNING SignalID INTO newSignalID;
		INSERT INTO CheckOut (UserID, SignalID) VALUES (user_id, newSignalID);

		appSignalID = '#SIGNAL' || newSignalID::text;
		customAppSignalID = 'SIGNAL' || newSignalID::text;

		IF channel_count > 1 THEN
			appSignalID = appSignalID || '_' || chr(64 + channel);
			customAppSignalID = customAppSignalID || '_' || chr(64 + channel);
		END IF;

		IF signal_type = 0 THEN
			dataSize = 16;		-- analog signal
		ELSE
			dataSize = 1;		-- discrete signal
		END IF;

		INSERT INTO SignalInstance (SignalID, AppSignalID, CustomAppSignalID, Caption, DataSize, Action) VALUES (newSignalID, appSignalID,  customAppSignalID, customAppSignalID, dataSize, 1) RETURNING SignalInstanceID INTO newSignalInstanceID;

		UPDATE Signal SET CheckedOutInstanceID = newSignalInstanceID WHERE Signal.SignalID = newSignalID;

		os.ID = newSignalID;
		os.deleted = FALSE;
		os.checkedout = TRUE;
		os.action = 1;
		os.userID = user_id;
		os.errCode = 0;

		RETURN NEXT os;
	END LOOP;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION public.add_signal(integer, integer, integer)
  OWNER TO u7;
