-------------------------------------------------------------------------
--
-- Misprint fixing in field SpecPropStruct
--
-------------------------------------------------------------------------
UPDATE signalinstance SET specpropstruct =
        REPLACE(
		    REPLACE(
			    REPLACE(specpropstruct, ';HighEngeneeringUnits;', ';HighEngineeringUnits;'),
				    ';LowEngeneeringUnits;', ';LowEngineeringUnits;'),
					    ' engeneering ', ' engineering ')
WHERE specpropstruct != '';


-- Removed update of SignalPropertyBehavior.csv

SELECT 1;

---------------------------------------------------------------------------
--
-- Not used columns deleting
--
---------------------------------------------------------------------------

ALTER TABLE signalinstance
    DROP COLUMN IF EXISTS caption,
	DROP COLUMN IF EXISTS analogsignalformat,
	DROP COLUMN IF EXISTS datasize,
	DROP COLUMN IF EXISTS lowadc,
	DROP COLUMN IF EXISTS highadc,
	DROP COLUMN IF EXISTS lowengeneeringunits,
	DROP COLUMN IF EXISTS highengeneeringunits,
	DROP COLUMN IF EXISTS lowvalidrange,
	DROP COLUMN IF EXISTS highvalidrange,
	DROP COLUMN IF EXISTS electriclowlimit,
	DROP COLUMN IF EXISTS electrichighlimit,
	DROP COLUMN IF EXISTS electricunit,
	DROP COLUMN IF EXISTS sensortype,
	DROP COLUMN IF EXISTS acquire,
	DROP COLUMN IF EXISTS decimalplaces,
	DROP COLUMN IF EXISTS coarseaperture,
	DROP COLUMN IF EXISTS outputmode,
	DROP COLUMN IF EXISTS filteringtime,
	DROP COLUMN IF EXISTS spreadtolerance,
	DROP COLUMN IF EXISTS byteorder,
	DROP COLUMN IF EXISTS enabletuning,
	DROP COLUMN IF EXISTS tuningdefaultdouble,
	DROP COLUMN IF EXISTS tuninglowbounddouble,
	DROP COLUMN IF EXISTS tuninghighbounddouble,
	DROP COLUMN IF EXISTS bustypeid,
	DROP COLUMN IF EXISTS adaptiveaperture,
	DROP COLUMN IF EXISTS unit,
	DROP COLUMN IF EXISTS fineaperture,
	DROP COLUMN IF EXISTS tuningdefaultint,
	DROP COLUMN IF EXISTS tuninglowboundint,
	DROP COLUMN IF EXISTS tuninghighboundint,
	DROP COLUMN IF EXISTS archive;

CREATE OR REPLACE FUNCTION public.add_signal(
    user_id integer,
	signal_type integer,
	channel_count integer)
	RETURNS SETOF objectstate
	LANGUAGE 'plpgsql'

    COST 100
	VOLATILE
	ROWS 1000
AS $BODY$
DECLARE
    newGroupID integer;
	channel integer;
	newSignalID integer;
	newSignalInstanceID integer;
	appSignalID varchar;
	customAppSignalID varchar;
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

        INSERT INTO SignalInstance (SignalID, AppSignalID, CustomAppSignalID, Action) VALUES (newSignalID, appSignalID,  customAppSignalID, 1) RETURNING SignalInstanceID INTO newSignalInstanceID;

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
$BODY$;

