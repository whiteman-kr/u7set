-- Add new columns into signalInstance table

ALTER TABLE public.signalinstance RENAME COLUMN tuningdefaultvalue TO tuningdefaultdouble;
ALTER TABLE public.signalinstance RENAME COLUMN tuninglowbound TO tuninglowbounddouble;
ALTER TABLE public.signalinstance RENAME COLUMN tuninghighbound TO tuninghighbounddouble;

ALTER TABLE public.signalinstance ADD COLUMN tuningdefaultint bigint NOT NULL DEFAULT 0;
ALTER TABLE public.signalinstance ADD COLUMN tuninglowboundint bigint NOT NULL DEFAULT 0;
ALTER TABLE public.signalinstance ADD COLUMN tuninghighboundint bigint NOT NULL DEFAULT 0;

-- Copy values from tuningdefaultvalue, tuninglowbound, tuninghighbound into new columns

UPDATE public.signalinstance SET    tuningdefaultint = tuningdefaultdouble,
                                    tuninglowboundint = tuninglowbounddouble,
				    tuninghighboundint = tuninghighbounddouble WHERE signalinstance.enabletuning = TRUE;

-- Drop unnecessary columns

ALTER TABLE public.signalinstance DROP COLUMN tuningdefaultvalue;
ALTER TABLE public.signalinstance DROP COLUMN tuninglowbound;
ALTER TABLE public.signalinstance DROP COLUMN tuninghighbound;

-- Drop all stored procedures dependent from SignalData type

DROP FUNCTION get_latest_signal(integer, integer);
DROP FUNCTION set_signal_workcopy(integer, signaldata);
DROP FUNCTION get_latest_signals(integer, integer[]);
DROP FUNCTION get_latest_signals_all(integer);
DROP FUNCTION checkout_signals(integer, integer[]);
DROP FUNCTION get_specific_signal(integer,integer,integer);
DROP FUNCTION get_latest_signals_by_appsignalids(integer,text[]);

-- Drop SignalData type

DROP TYPE signaldata;

-- Create new SignalData type

CREATE TYPE public.signaldata AS
(
    appsignalid text,
    customappsignalid text,
    caption text,
    equipmentid text,
    bustypeid text,
    channel integer,

    signaltype integer,
    inouttype integer,

    datasize integer,
    byteorder integer,

    analogsignalformat integer,
    unit text,

    lowadc integer,
    highadc integer,
    lowengeneeringunits double precision,
    highengeneeringunits double precision,
    lowvalidrange double precision,
    highvalidrange double precision,
    filteringtime double precision,
    spreadtolerance double precision,

    electriclowlimit double precision,
    electrichighlimit double precision,
    electricunit integer,
    sensortype integer,
    outputmode integer,

    enabletuning boolean,

    tuningdefaultdouble double precision,
    tuninglowbounddouble double precision,
    tuninghighbounddouble double precision,

    tuningdefaultfloat real,
    tuninglowboundfloat real,
    tuninghighboundfloat real,

    tuningdefaultint64 bigint,
    tuninglowboundint64 bigint,
    tuninghighboundint64 bigint,

    acquire boolean,
    decimalplaces integer,
    coarseaperture double precision,
    fineaperture double precision,
    adaptiveaperture boolean,

    signalid integer,
    signalgroupid integer,
    signalinstanceid integer,
    changesetid integer,
    checkedout boolean,
    userid integer,
    created timestamp with time zone,
    deleted boolean,
    instancecreated timestamp with time zone,
    instanceaction integer
);

-- Create new versions of stored procedures

CREATE OR REPLACE FUNCTION public.get_latest_signal(
    user_id integer,
    signal_id integer)
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
        signal_data signaldata;
	chInInstanceID integer;
	chOutInstanceID integer;
	instanceID integer;
	userIsAdmin boolean;
	checkOutUserID integer;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
	THEN
	        RETURN;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID INTO chInInstanceID, chOutInstanceID, checkOutUserID
	FROM Signal AS S
	WHERE S.SignalID = signal_id;

        IF chOutInstanceID IS NULL THEN
	        -- has not checkOuts for the signal
		instanceID = chInInstanceID;
	ELSE
	        -- has checkOuts for the signal
		IF checkOutUserID = user_id OR userIsAdmin THEN
		        instanceID = chOutInstanceID;
		ELSE
		        instanceID = chInInstanceID;
		END IF;
	END IF;

        RETURN QUERY SELECT
	        SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.EquipmentID,
		SI.BusTypeID,
		S.Channel,

                S.Type,
		SI.InOutType,

                SI.DataSize,
		SI.ByteOrder,

                SI.AnalogSignalFormat,
		SI.Unit,

                SI.LowADC,
		SI.HighADC,
		SI.LowengEneeringUnits,
		SI.HighEngeneeringUnits,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.FilteringTime,
		SI.SpreadTolerance,

                SI.ElectricLowLimit,
		SI.ElectricHighLimit,
		SI.ElectricUnit,
		SI.SensorType,
		SI.OutputMode,

                SI.EnableTuning,

                SI.TuningDefaultDouble,
		SI.TuningLowBoundDouble,
		SI.TuningHighBoundDouble,

                SI.TuningDefaultFloat,
		SI.TuningLowBoundFloat,
		SI.TuningHighBoundFloat,

                SI.TuningDefaultInt64,
		SI.TuningLowBoundInt64,
		SI.TuningHighBoundInt64,

                SI.Acquire,
		SI.DecimalPlaces,
		SI.CoarseAperture,
		SI.FineAperture,
		SI.AdaptiveAperture,

                S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(checkedOutInstanceID IS NOT NULL),	-- CheckedOut
		checkOutUserID,				-- signal checked out for user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	        SI.SignalID = signal_id AND
		SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.SignalInstanceID = instanceID;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;


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

        findAppSignalID integer [] = ARRAY(SELECT * FROM get_signal_ids_with_appsignalid(user_id, sd.AppSignalID));

        findCustomAppSignalID integer [] = ARRAY(SELECT * FROM get_signal_ids_with_customappsignalid(user_id, sd.CustomAppSignalID));
BEGIN
        SELECT
	        S.CheckedOutInstanceID, S.UserID, S.SignalID INTO chOutInstanceID, chOutUserID, signalID
	FROM
	        Signal AS S
	WHERE
	        S.SignalID = sd.SignalID;

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
			Caption = sd.Caption,
			EquipmentID = sd.EquipmentID,
			BusTypeID = sd.BusTypeID,
			-- Channel is not updatable

                        -- SignalType is not updatable
			InOutType = sd.InOutType,

                        DataSize = sd.DataSize,
			ByteOrder = sd.ByteOrder,

                        AnalogSignalFormat = sd.AnalogSignalFormat,
			Unit = sd.Unit,

                        LowADC = sd.LowADC,
			HighADC = sd.HighADC,
			LowEngeneeringUnits = sd.LowEngeneeringUnits,
			HighEngeneeringUnits = sd.HighEngeneeringUnits,
			LowValidRange = sd.LowValidRange,
			HighValidRange = sd.HighValidRange,
			FilteringTime = sd.FilteringTime,
			SpreadTolerance = sd.SpreadTolerance,

                        ElectricLowLimit = sd.ElectricLowLimit,
			ElectricHighLimit = sd.ElectricHighLimit,
			ElectricUnit = sd.ElectricUnit,
			SensorType = sd.SensorType,
			OutputMode = sd.OutputMode,

                        EnableTuning = sd.EnableTuning,
			TuningDefaultValue = sd.TuningDefaultValue,
			TuningLowBound = sd.TuningLowBound,
			TuningHighBound = sd.TuningHighBound,

                        Acquire = sd.Acquire,
			DecimalPlaces = sd.DecimalPlaces,
			CoarseAperture = sd.CoarseAperture,
			FineAperture = sd.FineAperture,
			AdaptiveAperture = sd.AdaptiveAperture

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

CREATE OR REPLACE FUNCTION public.get_latest_signals(
    user_id integer,
    signal_ids integer[])
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
        userIsAdmin boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
	THEN
	        RETURN;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        RETURN QUERY SELECT
	        SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.EquipmentID,
		SI.BusTypeID,
		S.Channel,

                S.Type,
		SI.InOutType,

                SI.DataSize,
		SI.ByteOrder,

                SI.AnalogSignalFormat,
		SI.Unit,

                SI.LowADC,
		SI.HighADC,
		SI.LowengEneeringUnits,
		SI.HighEngeneeringUnits,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.FilteringTime,
		SI.SpreadTolerance,

                SI.ElectricLowLimit,
		SI.ElectricHighLimit,
		SI.ElectricUnit,
		SI.SensorType,
		SI.OutputMode,

                SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,

                SI.Acquire,
		SI.DecimalPlaces,
		SI.CoarseAperture,
		SI.FineAperture,
		SI.AdaptiveAperture,

                S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
		S.UserID,				-- signal checked out for user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	        SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.SignalInstanceID IN (

                        SELECT
			        SG.CheckedInInstanceID
			FROM
			        Signal AS SG
			WHERE
			        (SG.CheckedOutInstanceID IS NULL OR
				(SG.UserID <> user_id AND userIsAdmin = FALSE))
				        AND
				SG.SignalID = ANY(signal_ids)

                                UNION ALL

                        SELECT
			        SG.CheckedOutInstanceID
			FROM
			        Signal AS SG
			WHERE
			        (SG.CheckedOutInstanceID IS NOT NULL AND
				(SG.UserID = user_id OR userIsAdmin = TRUE))
				        AND
				SG.SignalID = ANY(signal_ids)
			)
	ORDER BY S.SignalID;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;

CREATE OR REPLACE FUNCTION public.get_latest_signals_all(user_id integer)
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
        userIsAdmin boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
	THEN
	        RETURN;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        RETURN QUERY SELECT
	        SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.EquipmentID,
		SI.BusTypeID,
		S.Channel,

                S.Type,
		SI.InOutType,

                SI.DataSize,
		SI.ByteOrder,

                SI.AnalogSignalFormat,
		SI.Unit,

                SI.LowADC,
		SI.HighADC,
		SI.LowengEneeringUnits,
		SI.HighEngeneeringUnits,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.FilteringTime,
		SI.SpreadTolerance,

                SI.ElectricLowLimit,
		SI.ElectricHighLimit,
		SI.ElectricUnit,
		SI.SensorType,
		SI.OutputMode,

                SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,

                SI.Acquire,
		SI.DecimalPlaces,
		SI.CoarseAperture,
		SI.FineAperture,
		SI.AdaptiveAperture,

                S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
		S.UserID,				-- signal checked out for user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	        SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.SignalInstanceID IN (

                        SELECT
			        SG.CheckedInInstanceID
			FROM
			        Signal AS SG
			WHERE
			        (SG.CheckedOutInstanceID IS NULL OR
				(SG.UserID <> user_id AND userIsAdmin = FALSE))

                                UNION ALL

                        SELECT
			        SG.CheckedOutInstanceID
			FROM
			        Signal AS SG
			WHERE
			        (SG.CheckedOutInstanceID IS NOT NULL AND
				(SG.UserID = user_id OR userIsAdmin = TRUE))
			)
	ORDER BY S.SignalID;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;

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
				os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
				RETURN NEXT os;

                        ELSE
			        -- add record to the CheckOut table
				INSERT INTO CheckOut (UserID, SignalID) VALUES (user_id, signal_id);

                                -- make new signal workcopy in SignalInstance
				INSERT INTO
				        SignalInstance (
					        AppSignalID,
						CustomAppSignalID,
						Caption,
						EquipmentID,
						BusTypeID,

                                                InOutType,

                                                DataSize,
						ByteOrder,

                                                AnalogSignalFormat,
						Unit,

                                                LowADC,
						HighADC,
						LowengEneeringUnits,
						HighEngeneeringUnits,
						LowValidRange,
						HighValidRange,
						FilteringTime,
						SpreadTolerance,

                                                ElectricLowLimit,
						ElectricHighLimit,
						ElectricUnit,
						SensorType,
						OutputMode,

                                                EnableTuning,
						TuningDefaultValue,
						TuningLowBound,
						TuningHighBound,

                                                Acquire,
						DecimalPlaces,
						CoarseAperture,
						FineAperture,
						AdaptiveAperture,

                                                SignalID,
						Action)
					SELECT
					        AppSignalID,
						CustomAppSignalID,
						Caption,
						EquipmentID,
						BusTypeID,

                                                InOutType,

                                                DataSize,
						ByteOrder,

                                                AnalogSignalFormat,
						Unit,

                                                LowADC,
						HighADC,
						LowengEneeringUnits,
						HighEngeneeringUnits,
						LowValidRange,
						HighValidRange,
						FilteringTime,
						SpreadTolerance,

                                                ElectricLowLimit,
						ElectricHighLimit,
						ElectricUnit,
						SensorType,
						OutputMode,

                                                EnableTuning,
						TuningDefaultValue,
						TuningLowBound,
						TuningHighBound,

                                                Acquire,
						DecimalPlaces,
						CoarseAperture,
						FineAperture,
						AdaptiveAperture,

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
  COST 100
  ROWS 1000;


CREATE OR REPLACE FUNCTION public.get_specific_signal(
    user_id integer,
    signal_id integer,
    changeset_id integer)
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
    actual_changeset_id integer;

BEGIN
    IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
        THEN
	        RETURN;
	END IF;

        actual_changeset_id := (SELECT max(ChangesetID) FROM SignalInstance WHERE SignalID = signal_id AND ChangesetID <= changeset_id);

    RETURN QUERY
        SELECT
	                SI.AppSignalID,
			SI.CustomAppSignalID,
			SI.Caption,
			SI.EquipmentID,
			SI.BusTypeID,
			S.Channel,

                        S.Type,
			SI.InOutType,

                        SI.DataSize,
			SI.ByteOrder,

                        SI.AnalogSignalFormat,
			SI.Unit,

                        SI.LowADC,
			SI.HighADC,
			SI.LowengEneeringUnits,
			SI.HighEngeneeringUnits,
			SI.LowValidRange,
			SI.HighValidRange,
			SI.FilteringTime,
			SI.SpreadTolerance,

                        SI.ElectricLowLimit,
			SI.ElectricHighLimit,
			SI.ElectricUnit,
			SI.SensorType,
			SI.OutputMode,

                        SI.EnableTuning,
			SI.TuningDefaultValue,
			SI.TuningLowBound,
			SI.TuningHighBound,

                        SI.Acquire,
			SI.DecimalPlaces,
			SI.CoarseAperture,
			SI.FineAperture,
			SI.AdaptiveAperture,

                        S.SignalID,
			S.SignalGroupID,
			SI.SignalInstanceID,
			SI.ChangesetID,
			(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
			CHS.UserID,				-- signal checked out for user with user_id
			S.Created,
			S.Deleted,
			SI.Created,				-- InstanceCreated timestamp with time zone,
			SI.Action				-- InstanceAction
		FROM
		    Signal AS S,
		        SignalInstance AS SI,
			Changeset AS CHS
		WHERE
		    SI.SignalID = signal_id AND
		        SI.SignalID = S.SignalID AND
			SI.ChangesetID = actual_changeset_id AND
			CHS.ChangesetID = actual_changeset_id;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;

CREATE OR REPLACE FUNCTION public.get_latest_signals_by_appsignalids(
    user_id integer,
    appsignal_ids text[])
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
        userIsAdmin boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
	THEN
	        RETURN;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        RETURN QUERY SELECT
	                SI.AppSignalID,
			SI.CustomAppSignalID,
			SI.Caption,
			SI.EquipmentID,
			SI.BusTypeID,
			S.Channel,

                        S.Type,
			SI.InOutType,

                        SI.DataSize,
			SI.ByteOrder,

                        SI.AnalogSignalFormat,
			SI.Unit,

                        SI.LowADC,
			SI.HighADC,
			SI.LowengEneeringUnits,
			SI.HighEngeneeringUnits,
			SI.LowValidRange,
			SI.HighValidRange,
			SI.FilteringTime,
			SI.SpreadTolerance,

                        SI.ElectricLowLimit,
			SI.ElectricHighLimit,
			SI.ElectricUnit,
			SI.SensorType,
			SI.OutputMode,

                        SI.EnableTuning,
			SI.TuningDefaultValue,
			SI.TuningLowBound,
			SI.TuningHighBound,

                        SI.Acquire,
			SI.DecimalPlaces,
			SI.CoarseAperture,
			SI.FineAperture,
			SI.AdaptiveAperture,

                        S.SignalID,
			S.SignalGroupID,
			SI.SignalInstanceID,
			SI.ChangesetID,
			(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
			S.UserID,				-- signal checked out for user with user_id
			S.Created,
			S.Deleted,
			SI.Created,				-- InstanceCreated timestamp with time zone,
			SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	        SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.SignalInstanceID IN (

                        SELECT
			        SG.CheckedInInstanceID
			FROM
			        Signal AS SG, SignalInstance AS SI
			WHERE
			        (SG.SignalID = SI.SignalID) AND
				(SG.CheckedOutInstanceID IS NULL OR
				(SG.UserID <> user_id AND userIsAdmin = FALSE))
				        AND
				SI.AppSignalID = ANY(appsignal_ids)

                                UNION ALL

                        SELECT
			        SG.CheckedOutInstanceID
			FROM
			        Signal AS SG, SignalInstance AS SI
			WHERE
			        (SG.SignalID = SI.SignalID) AND
				(SG.CheckedOutInstanceID IS NOT NULL AND
				(SG.UserID = user_id OR userIsAdmin = TRUE))
				        AND
				SI.AppSignalID = ANY(appsignal_ids)
			)
	ORDER BY S.SignalID;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;

