-- Add column unit into signalInstance table

ALTER TABLE public.signalinstance ADD COLUMN unit text;

-- Set values of column unit from unit table

UPDATE signalInstance AS si SET (unit) = (SELECT unit_ru FROM unit WHERE unitid = si.unitid);

-- Drop constraints to unit table

ALTER TABLE public.signalinstance DROP CONSTRAINT inputunit_fkey;
ALTER TABLE public.signalinstance DROP CONSTRAINT outputunit_fkey;
ALTER TABLE public.signalinstance DROP CONSTRAINT unit_fkey;

-- Make other changes in signalInstance table

ALTER TABLE public.signalinstance DROP COLUMN unitid;
ALTER TABLE public.signalInstance RENAME COLUMN dataformatid TO analogsignalformat;
ALTER TABLE public.signalInstance DROP COLUMN adjustment;
ALTER TABLE public.signalInstance DROP COLUMN unbalancelimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputlowlimit TO electriclowlimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputhighlimit TO electrichighlimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputunitid TO electricunit;
ALTER TABLE public.signalInstance RENAME COLUMN inputsensorid TO sensortype;
ALTER TABLE public.signalInstance DROP COLUMN outputlowlimit;
ALTER TABLE public.signalInstance DROP COLUMN outputhighlimit;
ALTER TABLE public.signalInstance DROP COLUMN outputunitid;
ALTER TABLE public.signalInstance DROP COLUMN outputsensorid;
ALTER TABLE public.signalInstance DROP COLUMN calculated;
ALTER TABLE public.signalInstance DROP COLUMN normalstate;
ALTER TABLE public.signalInstance RENAME COLUMN aperture TO coarseaperture;
ALTER TABLE public.signalinstance ADD COLUMN fineaperture double precision NOT NULL DEFAULT 0.5;
ALTER TABLE public.signalInstance RENAME COLUMN outputrangemode TO outputmode;

-- Convert  electricunit (old inputunitid) from unitID to E::ElectricUnit values

UPDATE signalInstance SET electricunit = 0 WHERE electricunit = 1;
UPDATE signalInstance SET electricunit = 1 WHERE electricunit = 15;
UPDATE signalInstance SET electricunit = 2 WHERE electricunit = 11;
UPDATE signalInstance SET electricunit = 3 WHERE electricunit = 20;
UPDATE signalInstance SET electricunit = 4 WHERE electricunit = 12;

UPDATE signalInstance SET electricunit = 0 WHERE electricunit > 4;	    -- all other set to E::ElectricUnit::NoUnit

-- Set value of fineAperture as half of coarseAperture

UPDATE signalInstance SET fineaperture = coarseaperture / 2;

-- Drop unit table

DROP TABLE public.unit;

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
    tuningdefaultvalue double precision,
    tuninglowbound double precision,
    tuninghighbound double precision,

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

