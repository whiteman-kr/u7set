------------------------------------------------------------------------------
--
-- Add column OutputRangeMode, FilteringTime, MaxDifference to SignalInstance table
-- Modification of the dependent types & stored procedures
--
------------------------------------------------------------------------------

ALTER TABLE signalinstance ADD COLUMN outputrangemode integer NOT NULL DEFAULT 1;

ALTER TABLE signalinstance ADD COLUMN filteringtime double precision NOT NULL DEFAULT 0.05;

ALTER TABLE signalinstance ADD COLUMN maxdifference double precision NOT NULL DEFAULT 0.5;

ALTER TABLE signalinstance ADD COLUMN byteorder integer NOT NULL DEFAULT 1;


DROP FUNCTION set_signal_workcopy(integer, signaldata);
DROP FUNCTION checkout_signals(user_id integer, signal_ids integer[]);

DROP TYPE signaldata;


CREATE TYPE signaldata AS
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
	strid text,
	extstrid text,
	name text,
	dataformatid integer,
	datasize integer,
	lowadc integer,
	highadc integer,
	lowlimit double precision,
	highlimit double precision,
	unitid integer,
	adjustment double precision,
	droplimit double precision,
	excesslimit double precision,
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
	devicestrid text,
	outputrangemode integer,
	filteringtime double precision,
	maxdifference double precision,
	byteorder integer);


CREATE OR REPLACE FUNCTION get_latest_signal(user_id integer, signal_id integer)
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
		SI.StrId,
		SI.ExtStrId,
		SI.Name,
		SI.DataFormatID,
		SI.DataSize,
		SI.LowADC,
		SI.HighADC,
		SI.LowLimit,
		SI.HighLimit,
		SI.UnitID,
		SI.Adjustment,
		SI.DropLimit,
		SI.ExcessLimit,
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
		SI.DeviceStrID,
		SI.OutputRangeMode,
		SI.FilteringTime,
		SI.MaxDifference,
		SI.ByteOrder
	FROM Signal AS S, SignalInstance AS SI
	WHERE
		SI.SignalID = signal_id AND
		SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.SignalInstanceID = instanceID;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;



CREATE OR REPLACE FUNCTION set_signal_workcopy(user_id integer, sd signaldata)
  RETURNS objectstate AS
$BODY$
DECLARE
	userIsAdmin boolean;
	chOutInstanceID integer;
	chOutUserID integer;
	os objectstate;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	SELECT S.CheckedOutInstanceID, S.UserID
	INTO chOutInstanceID, chOutUserID
	FROM Signal AS S
	WHERE S.SignalID = sd.SignalID;

	IF (chOutInstanceID IS NOT NULL) AND
		(chOutUserID IS NOT NULL) AND
		(chOutUserID = user_id OR userIsAdmin)
	THEN
		-- update checked out workcopy
		UPDATE SignalInstance SET
			StrId = sd.StrID,
			ExtStrId = sd.ExtStrId,
			Name = sd.Name,
			DataFormatID = sd.DataFormatID,
			DataSize = sd.DataSize,
			LowADC = sd.LowADC,
			HighADC = sd.HighADC,
			LowLimit = sd.LowLimit,
			HighLimit = sd.HighLimit,
			UnitID = sd.UnitID,
			Adjustment = sd.Adjustment,
			DropLimit = sd.DropLimit,
			ExcessLimit = sd.ExcessLimit,
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
			DeviceStrID = sd.DeviceStrID,
			OutputRangeMode = sd.OutputRangeMode,
			FilteringTime = sd.FilteringTime,
			MaxDifference = sd.MaxDifference,
			ByteOrder = sd.ByteOrder
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


CREATE OR REPLACE FUNCTION checkout_signals(user_id integer, signal_ids integer[])
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
						StrId,
						ExtStrId,
						Name,
						DataFormatID,
						DataSize,
						LowADC,
						HighADC,
						LowLimit,
						HighLimit,
						UnitID,
						Adjustment,
						DropLimit,
						ExcessLimit,
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
						DeviceStrID,
						OutputRangeMode,
						FilteringTime,
						MaxDifference,
						ByteOrder )
					SELECT
						SI.SignalID,
						2,							-- Action Edit
						StrId,
						ExtStrId,
						Name,
						DataFormatID,
						DataSize,
						LowADC,
						HighADC,
						LowLimit,
						HighLimit,
						UnitID,
						Adjustment,
						DropLimit,
						ExcessLimit,
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
						DeviceStrID,
						OutputRangeMode,
						FilteringTime,
						MaxDifference,
						ByteOrder
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

