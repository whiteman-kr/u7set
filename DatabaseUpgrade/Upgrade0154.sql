--
-- RPCT-1632, RPCT-1655 - append new fields in SignalInstance table
--

-- Append TuningLowBound field

ALTER TABLE public.signalinstance ADD COLUMN tuninglowbound double precision;
ALTER TABLE public.signalinstance ALTER COLUMN tuninglowbound SET NOT NULL;
ALTER TABLE public.signalinstance ALTER COLUMN tuninglowbound SET DEFAULT 0;

-- Append TuningHighBound field

ALTER TABLE public.signalinstance ADD COLUMN tuninghighbound double precision;
ALTER TABLE public.signalinstance ALTER COLUMN tuninghighbound SET NOT NULL;
ALTER TABLE public.signalinstance ALTER COLUMN tuninghighbound SET DEFAULT 100;

-- Append BusTypeID field

ALTER TABLE public.signalinstance ADD COLUMN bustypeid text;

-- Append AdaptiveAperture field

ALTER TABLE public.signalinstance ADD COLUMN adaptiveaperture boolean;
ALTER TABLE public.signalinstance ALTER COLUMN adaptiveaperture SET NOT NULL;
ALTER TABLE public.signalinstance ALTER COLUMN adaptiveaperture SET DEFAULT false;

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
   (signalid integer,
    signalgroupid integer,
    signalinstanceid integer,
    changesetid integer,
    checkedout boolean,
    userid integer,
    channel integer,
    type integer,
    created timestamp with time zone,
    deleted boolean,
    instancecreated timestamp with time zone,
    action integer,
    appsignalid text,
    customappsignalid text,
    caption text,
    dataformatid integer,
    datasize integer,
    lowadc integer,
    highadc integer,
    lowengeneeringunits double precision,
    highengeneeringunits double precision,
    unitid integer,
    adjustment double precision,
    lowvalidrange double precision,
    highvalidrange double precision,
    unbalancelimit double precision,
    inputlowlimit double precision,
    inputhighlimit double precision,
    inputunitid integer,
    inputsensorid integer,
    outputlowlimit double precision,
    outputhighlimit double precision,
    outputunitid integer,
    outputsensorid integer,
    acquire boolean,
    calculated boolean,
    normalstate integer,
    decimalplaces integer,
    aperture double precision,
    inouttype integer,
    equipmentid text,
    outputrangemode integer,
    filteringtime double precision,
    spreadtolerance double precision,
    byteorder integer,
    enabletuning boolean,
    tuningdefaultvalue double precision,
    tuninglowbound double precision,
    tuninghighbound double precision,
    bustypeid text,
    adaptiveaperture boolean);

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
	        S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(checkedOutInstanceID IS NOT NULL),
		checkOutUserID,				-- signal checked out for user with user_id
		S.Channel,
		S.Type,
		S.Created,
		S.Deleted,
		SI.Created,					-- instancecreated timestamp with time zone,
		SI.Action,
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.DataFormatID,
		SI.DataSize,
		SI.LowADC,
		SI.HighADC,
		SI.LowengEneeringUnits,
		SI.HighEngeneeringUnits,
		SI.UnitID,
		SI.Adjustment,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.UnbalanceLimit,
		SI.InputLowLimit,
		SI.InputHighLimit,
		SI.InputUnitID,
		SI.InputSensorID,
		SI.OutputLowLimit,
		SI.OutputHighLimit,
		SI.OutputUnitID,
		SI.OutputSensorID,
		SI.Acquire,
		SI.Calculated,
		SI.NormalState,
		SI.DecimalPlaces,
		SI.Aperture,
		SI.InOutType,
		SI.EquipmentID,
		SI.OutputRangeMode,
		SI.FilteringTime,
		SI.SpreadTolerance,
		SI.ByteOrder,
		SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,
		SI.BusTypeID,
		SI.AdaptiveAperture
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
			DataFormatID = sd.DataFormatID,
			DataSize = sd.DataSize,
			LowADC = sd.LowADC,
			HighADC = sd.HighADC,
			LowEngeneeringUnits = sd.LowEngeneeringUnits,
			HighEngeneeringUnits = sd.HighEngeneeringUnits,
			UnitID = sd.UnitID,
			Adjustment = sd.Adjustment,
			LowValidRange = sd.LowValidRange,
			HighValidRange = sd.HighValidRange,
			UnbalanceLimit = sd.UnbalanceLimit,
			InputLowLimit = sd.InputLowLimit,
			InputHighLimit = sd.InputHighLimit,
			InputUnitID = sd.InputUnitID,
			InputSensorID = sd.InputSensorID,
			OutputLowLimit = sd.OutputLowLimit,
			OutputHighLimit = sd.OutputHighLimit,
			OutputUnitID = sd.OutputUnitID,
			OutputSensorID = sd.OutputSensorID,
			Acquire = sd.Acquire,
			Calculated = sd.Calculated,
			NormalState = sd.NormalState,
			DecimalPlaces = sd.DecimalPlaces,
			Aperture = sd.Aperture,
			InOutType = sd.InOutType,
			EquipmentID = sd.EquipmentID,
			OutputRangeMode = sd.OutputRangeMode,
			FilteringTime = sd.FilteringTime,
			SpreadTolerance = sd.SpreadTolerance,
			ByteOrder = sd.ByteOrder,
			EnableTuning = sd.EnableTuning,
			TuningDefaultValue = sd.TuningDefaultValue,
			TuningLowBound = sd.TuningLowBound,
			TuningHighBound = sd.TuningHighBound,
			BusTypeID = sd.BusTypeID,
			AdaptiveAperture = sd.AdaptiveAperture
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
	        S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(checkedOutInstanceID IS NOT NULL),
		S.UserID,					-- signal checked out for user with user_id
		S.Channel,
		S.Type,
		S.Created,
		S.Deleted,
		SI.Created,					-- instancecreated timestamp with time zone,
		SI.Action,
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.DataFormatID,
		SI.DataSize,
		SI.LowADC,
		SI.HighADC,
		SI.LowEngeneeringUnits,
		SI.HighEngeneeringUnits,
		SI.UnitID,
		SI.Adjustment,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.UnbalanceLimit,
		SI.InputLowLimit,
		SI.InputHighLimit,
		SI.InputUnitID,
		SI.InputSensorID,
		SI.OutputLowLimit,
		SI.OutputHighLimit,
		SI.OutputUnitID,
		SI.OutputSensorID,
		SI.Acquire,
		SI.Calculated,
		SI.NormalState,
		SI.DecimalPlaces,
		SI.Aperture,
		SI.InOutType,
		SI.EquipmentID,
		SI.OutputRangeMode,
		SI.FilteringTime,
		SI.SpreadTolerance,
		SI.ByteOrder,
		SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,
		SI.BusTypeID,
		SI.AdaptiveAperture
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
	        S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(checkedOutInstanceID IS NOT NULL),
		S.UserID,					-- signal checked out for user with user_id
		S.Channel,
		S.Type,
		S.Created,
		S.Deleted,
		SI.Created,					-- instancecreated timestamp with time zone,
		SI.Action,
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.DataFormatID,
		SI.DataSize,
		SI.LowADC,
		SI.HighADC,
		SI.LowEngeneeringUnits,
		SI.HighEngeneeringUnits,
		SI.UnitID,
		SI.Adjustment,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.UnbalanceLimit,
		SI.InputLowLimit,
		SI.InputHighLimit,
		SI.InputUnitID,
		SI.InputSensorID,
		SI.OutputLowLimit,
		SI.OutputHighLimit,
		SI.OutputUnitID,
		SI.OutputSensorID,
		SI.Acquire,
		SI.Calculated,
		SI.NormalState,
		SI.DecimalPlaces,
		SI.Aperture,
		SI.InOutType,
		SI.EquipmentID,
		SI.OutputRangeMode,
		SI.FilteringTime,
		SI.SpreadTolerance,
		SI.ByteOrder,
		SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,
		SI.BusTypeID,
		SI.AdaptiveAperture
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
					        SignalID,
						Action,
						AppSignalID,
						CustomAppSignalID,
						Caption,
						DataFormatID,
						DataSize,
						LowADC,
						HighADC,
						LowEngeneeringUnits,
						HighEngeneeringUnits,
						UnitID,
						Adjustment,
						LowValidRange,
						HighValidRange,
						UnbalanceLimit,
						InputLowLimit,
						InputHighLimit,
						InputUnitID,
						InputSensorID,
						OutputLowLimit,
						OutputHighLimit,
						OutputUnitID,
						OutputSensorID,
						Acquire,
						Calculated,
						NormalState,
						DecimalPlaces,
						Aperture,
						InOutType,
						EquipmentID,
						OutputRangeMode,
						FilteringTime,
						SpreadTolerance,
						ByteOrder,
						EnableTuning,
						TuningDefaultValue,
						TuningLowBound,
						TuningHighBound,
						BusTypeID,
						AdaptiveAperture)
					SELECT
					        SI.SignalID,
						2,							-- Action Edit
						AppSignalID,
						CustomAppSignalID,
						Caption,
						DataFormatID,
						DataSize,
						LowADC,
						HighADC,
						LowEngeneeringUnits,
						HighEngeneeringUnits,
						UnitID,
						Adjustment,
						LowValidRange,
						HighValidRange,
						UnbalanceLimit,
						InputLowLimit,
						InputHighLimit,
						InputUnitID,
						InputSensorID,
						OutputLowLimit,
						OutputHighLimit,
						OutputUnitID,
						OutputSensorID,
						Acquire,
						Calculated,
						NormalState,
						DecimalPlaces,
						Aperture,
						InOutType,
						EquipmentID,
						OutputRangeMode,
						FilteringTime,
						SpreadTolerance,
						ByteOrder,
						EnableTuning,
						TuningDefaultValue,
						TuningLowBound,
						TuningHighBound,
						BusTypeID,
						AdaptiveAperture
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
	            S.SignalID,
		        S.SignalGroupID,
			SI.SignalInstanceID,
			SI.ChangesetID,
			S.CheckedOutInstanceID IS NOT NULL,
			CHS.UserID,					-- signal checked out for user with user_id
			S.Channel,
			S.Type,
			S.Created,
			S.Deleted,
			SI.Created,					-- instancecreated timestamp with time zone,
			SI.Action,
			SI.AppSignalID,
			SI.CustomAppSignalID,
			SI.Caption,
			SI.DataFormatID,
			SI.DataSize,
			SI.LowADC,
			SI.HighADC,
			SI.LowengEneeringUnits,
			SI.HighEngeneeringUnits,
			SI.UnitID,
			SI.Adjustment,
			SI.LowValidRange,
			SI.HighValidRange,
			SI.UnbalanceLimit,
			SI.InputLowLimit,
			SI.InputHighLimit,
			SI.InputUnitID,
			SI.InputSensorID,
			SI.OutputLowLimit,
			SI.OutputHighLimit,
			SI.OutputUnitID,
			SI.OutputSensorID,
			SI.Acquire,
			SI.Calculated,
			SI.NormalState,
			SI.DecimalPlaces,
			SI.Aperture,
			SI.InOutType,
			SI.EquipmentID,
			SI.OutputRangeMode,
			SI.FilteringTime,
			SI.SpreadTolerance,
			SI.ByteOrder,
			SI.EnableTuning,
			SI.TuningDefaultValue,
			SI.TuningLowBound,
			SI.TuningHighBound,
			SI.BusTypeID,
			SI.AdaptiveAperture
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
	        S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(checkedOutInstanceID IS NOT NULL),
		S.UserID,					-- signal checked out for user with user_id
		S.Channel,
		S.Type,
		S.Created,
		S.Deleted,
		SI.Created,					-- instancecreated timestamp with time zone,
		SI.Action,
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.Caption,
		SI.DataFormatID,
		SI.DataSize,
		SI.LowADC,
		SI.HighADC,
		SI.LowEngeneeringUnits,
		SI.HighEngeneeringUnits,
		SI.UnitID,
		SI.Adjustment,
		SI.LowValidRange,
		SI.HighValidRange,
		SI.UnbalanceLimit,
		SI.InputLowLimit,
		SI.InputHighLimit,
		SI.InputUnitID,
		SI.InputSensorID,
		SI.OutputLowLimit,
		SI.OutputHighLimit,
		SI.OutputUnitID,
		SI.OutputSensorID,
		SI.Acquire,
		SI.Calculated,
		SI.NormalState,
		SI.DecimalPlaces,
		SI.Aperture,
		SI.InOutType,
		SI.EquipmentID,
		SI.OutputRangeMode,
		SI.FilteringTime,
		SI.SpreadTolerance,
		SI.ByteOrder,
		SI.EnableTuning,
		SI.TuningDefaultValue,
		SI.TuningLowBound,
		SI.TuningHighBound,
		SI.BusTypeID,
		SI.AdaptiveAperture
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
