-- Add rows CheckedInInstanceID, CheckedOutInstanceID, UserID in table Signal

ALTER TABLE signal ADD COLUMN checkedininstanceid integer;

ALTER TABLE signal
  ADD CONSTRAINT checkedininstanceid_fkey FOREIGN KEY (checkedininstanceid)
	  REFERENCES signalinstance (signalinstanceid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION;

ALTER TABLE signal ADD COLUMN checkedoutinstanceid integer;

ALTER TABLE signal
  ADD CONSTRAINT checkedoutinstanceid_fkey FOREIGN KEY (checkedoutinstanceid)
	  REFERENCES signalinstance (signalinstanceid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION;

ALTER TABLE signal ADD COLUMN userid integer;

ALTER TABLE signal
  ADD CONSTRAINT userid_fkey FOREIGN KEY (userid)
	  REFERENCES "User" ("UserID") MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION;

-- Add type SignalData

CREATE TYPE signaldata AS (
	signalid integer,
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
	deviceid integer,
	inoutno integer
);

-- Add stored procedures

CREATE OR REPLACE FUNCTION add_signal(user_id integer, signal_type integer, channel_count integer)
	RETURNS SETOF integer AS
$BODY$
DECLARE
	newGroupID int;
	channel int;
	newSignalID int;
	newSignalInstanceID int;
	strID varchar;
	extStrID varchar;
	dataSize int;
BEGIN
	IF channel_count < 1 THEN
		channel_count := 1;
	END IF;

	IF channel_count > 6 THEN
		RAISE 'Signal channelCount must be equal or less then 6';
	END IF;

	IF channel_count > 1 THEN
		INSERT INTO SignalGroup DEFAULT VALUES RETURNING SignalGroupID INTO newGroupID;
	ELSE
		-- all single-channel signals are placed in group 0
		newGroupID := 0;
	END IF;

	FOR channel IN 1..channel_count LOOP
		INSERT INTO Signal (SignalGroupID, Channel, Type, Deleted, UserID) VALUES (newGroupID, channel, signal_type, false, user_id) RETURNING SignalID INTO newSignalID;
		INSERT INTO CheckOut (UserID, SignalID) VALUES (user_id, newSignalID);

		strID := '#SIGNAL' || newSignalID::text;
		extStrID = 'SIGNAL' || newSignalID::text;

		IF channel_count > 1 THEN
			strID = strID || '_' || chr(64 + channel);
			extStrID = extStrID || '_' || chr(64 + channel);
		END IF;

		IF signal_type = 0 THEN
			dataSize = 16;		-- analog signal
		ELSE
			dataSize = 1;		-- discrete signal
		END IF;

		INSERT INTO SignalInstance (SignalID, StrID, ExtStrID, Name, DataSize) VALUES (newSignalID, strID,  extStrID, extStrID, dataSize) RETURNING SignalInstanceID INTO newSignalInstanceID;

		UPDATE Signal SET CheckedOutInstanceID = newSignalInstanceID WHERE Signal.SignalID = newSignalID;
	END LOOP;

	IF channel_count > 1 THEN
		RETURN QUERY SELECT SignalID FROM Signal WHERE SignalGroupID = newGroupID ORDER BY SignalID ASC;
	ELSE
		RETURN NEXT newSignalID;
	END IF;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION get_signals_ids(user_id integer, with_deleted boolean)
	RETURNS SETOF integer AS
$BODY$
DECLARE
BEGIN
	-- select IDs of signals
	-- that checked in and/or checked out by user_id
	-- Signal must have corresponding SignalInstance

	RETURN QUERY
		SELECT S.SignalID
		FROM
			Signal AS S
		WHERE
			((S.CheckedInInstanceID IS NOT NULL) OR (S.CheckedOutInstanceID IS NOT NULL AND S.UserID = user_id)) AND
			(S.Deleted != TRUE OR with_deleted);
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION checkin_signals(user_id integer, signal_ids integer[], checkin_comment text)
  RETURNS integer AS
$BODY$
DECLARE
	newChangesetID int;
	wasCheckedOut int;
	signal_id int;
	userIsAdmin boolean;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	-- check if signals really checked out
	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT count(*) INTO wasCheckedOut FROM CheckOut WHERE SignalID = signal_id AND (UserID = user_id OR userIsAdmin);
		IF (wasCheckedOut <> 1)	THEN
			RAISE 'Signal ID = % is not checked out by User ID = %', signal_id, user_id;
			RETURN FALSE;
		END IF;
	END LOOP;

	--
	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, FALSE) RETURNING ChangesetID INTO newChangesetID;

	UPDATE SignalInstance SET ChangesetID = newChangesetID WHERE SignalID = ANY(signal_ids) AND ChangesetID IS NULL;

	UPDATE Signal
	SET
		CheckedInInstanceID = CheckedOutInstanceID,
		CheckedOutInstanceID = NULL,
		UserID = NULL
	WHERE
		SignalID = ANY(signal_ids);

	DELETE FROM CheckOut WHERE SignalID = ANY(signal_ids);

	RETURN newChangesetID;
END;
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION checkout_signals(user_id integer, signal_ids integer[])
  RETURNS boolean AS
$BODY$
DECLARE
	alreadyCheckedOut integer;
	signal_id integer;
	chOutInstanceID integer;
BEGIN
	SELECT count(CheckOutID) INTO alreadyCheckedOut FROM CheckOut WHERE SignalID = ANY(signal_ids);

	IF (alreadyCheckedOut > 0) THEN
		RAISE 'One of signals already checked out';
		RETURN FALSE;
	END IF;

	FOREACH signal_id IN ARRAY signal_ids
	LOOP
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
				DeviceID,
				InOutNo )
			SELECT
				SI.SignalID,
				2,
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
				DeviceID,
				InOutNo
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
	END LOOP;

	RETURN TRUE;
END
$BODY$
  LANGUAGE plpgsql;


--DROP FUNCTION get_signal_workcopy(user_id integer, signal_id integer);

CREATE OR REPLACE FUNCTION get_latest_signal(user_id integer, signal_id integer)
	RETURNS signaldata AS
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

	SELECT
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
		SI.DeviceID,
		SI.InOutNo
	INTO
		signal_data
	FROM Signal AS S, SignalInstance AS SI
	WHERE
		SI.SignalID = signal_id AND
		SI.SignalID = S.SignalID AND
		SI.SignalInstanceID = instanceID;

	RETURN signal_data;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION set_signal_workcopy(user_id integer, sd signaldata)
	RETURNS boolean AS
$BODY$
DECLARE
	userIsAdmin boolean;
	chOutInstanceID integer;
	chOutUserID integer;
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
			DeviceID = sd.DeviceID,
			InOutNo = sd.InOutNo
		WHERE
			SignalInstanceID = chOutInstanceID;

			RETURN TRUE;
	ELSE
		-- signal is not checked out by this user
		RAISE 'Signal ID = % is not checked out be user ID = %', sd.SignalID, user_id;

		RETURN FALSE;
	END IF;
END
$BODY$
LANGUAGE plpgsql;
