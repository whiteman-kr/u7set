-- Add rows CheckedInInstanceID and CheckedOutInstanceID in table Signal

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

CREATE OR REPLACE FUNCTION add_signal(userid integer, signalType integer, channelcount integer)
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
	IF channelcount < 1 THEN
		channelcount := 1;
	END IF;

	IF channelcount > 6 THEN
		RAISE 'Signal channelCount must be equal or less then 6';
	END IF;

	IF channelcount > 1 THEN
		INSERT INTO SignalGroup DEFAULT VALUES RETURNING SignalGroupID INTO newGroupID;
	ELSE
		-- all single-channel signals are placed in group 0
		newGroupID := 0;
	END IF;

	FOR channel IN 1..channelcount LOOP
		INSERT INTO Signal (SignalGroupID, Channel, Type, Deleted) VALUES (newGroupID, channel, signalType, false) RETURNING SignalID INTO newSignalID;
		INSERT INTO CheckOut (UserID, SignalID) VALUES (userid, newSignalID);

		strID := '#SIGNAL' || newSignalID::text;
		extStrID = 'SIGNAL' || newSignalID::text;

		IF channelcount > 1 THEN
			strID = strID || '_' || chr(64 + channel);
			extStrID = extStrID || '_' || chr(64 + channel);
		END IF;

		IF signalType = 0 THEN
			dataSize = 16;		-- analog signal
		ELSE
			dataSize = 1;		-- discrete signal
		END IF;

		INSERT INTO SignalInstance (SignalID, StrID, ExtStrID, Name, DataSize) VALUES (newSignalID, strID,  extStrID, extStrID, dataSize) RETURNING SignalInstanceID INTO newSignalInstanceID;

		UPDATE Signal SET CheckedOutInstanceID = newSignalInstanceID WHERE Signal.SignalID = newSignalID;
	END LOOP;

	IF channelcount > 1 THEN
		RETURN QUERY SELECT SignalID AS newSignalID FROM Signal WHERE SignalGroupID = newGroupID ORDER BY SignalID ASC;
	ELSE
		RETURN NEXT newSignalID;
	END IF;
END
$BODY$
LANGUAGE plpgsql;
