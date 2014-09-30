DROP FUNCTION get_signals_ids(integer, boolean);

CREATE OR REPLACE FUNCTION get_signals_ids(user_id integer, with_deleted boolean)
  RETURNS SETOF integer AS
$BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
	-- select IDs of signals
	-- that checked in and/or checked out by user_id
	-- Signal must have corresponding SignalInstance
	SELECT is_admin(user_id) INTO userIsAdmin;

	RETURN QUERY
		SELECT S.SignalID
		FROM
			Signal AS S
		WHERE
			((S.CheckedInInstanceID IS NOT NULL) OR (S.CheckedOutInstanceID IS NOT NULL AND (S.UserID = user_id OR userIsAdmin))) AND
			(S.Deleted != TRUE OR with_deleted)
		ORDER BY
			S.SignalID ASC;
END
$BODY$
  LANGUAGE plpgsql;


DROP FUNCTION add_signal(integer, integer, integer);

CREATE OR REPLACE FUNCTION add_signal(user_id integer, signal_type integer, channel_count integer)
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	newGroupID integer;
	channel integer;
	newSignalID integer;
	newSignalInstanceID integer;
	strID varchar;
	extStrID varchar;
	dataSize integer;
	os objectstate;
BEGIN
	IF channel_count < 1 THEN
		channel_count = 1;
	END IF;

	IF channel_count > 6 THEN
		RAISE 'Signal channelCount must be equal or less then 6';
	END IF;

	IF channel_count > 1 THEN
		INSERT INTO SignalGroup DEFAULT VALUES RETURNING SignalGroupID INTO newGroupID;
	ELSE
		-- all single-channel signals are placed in group 0
		newGroupID = 0;
	END IF;

	FOR channel IN 1..channel_count LOOP
		INSERT INTO Signal (SignalGroupID, Channel, Type, Deleted, UserID) VALUES (newGroupID, channel, signal_type, false, user_id) RETURNING SignalID INTO newSignalID;
		INSERT INTO CheckOut (UserID, SignalID) VALUES (user_id, newSignalID);

		strID = '#SIGNAL' || newSignalID::text;
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

		INSERT INTO SignalInstance (SignalID, StrID, ExtStrID, Name, DataSize, Action) VALUES (newSignalID, strID,  extStrID, extStrID, dataSize, 1) RETURNING SignalInstanceID INTO newSignalInstanceID;

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
LANGUAGE plpgsql;


DROP FUNCTION checkin_signals(integer, integer[], text);

CREATE OR REPLACE FUNCTION checkin_signals(user_id integer, signal_ids integer[], checkin_comment text)
	RETURNS SETOF objectstate AS
$BODY$
DECLARE
	newChangesetID integer;
	signal_id integer;
	checkedInSignalCount integer;
	userIsAdmin boolean;
	signalDeleted boolean;
	os objectstate;
	chInInstanceID integer;
	chOutInstanceID integer;
	chUserID integer;
	chOutAction integer;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, FALSE) RETURNING ChangesetID INTO newChangesetID;

	checkedInSignalCount = 0;

	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT CheckedInInstanceID, CheckedOutInstanceID, UserID, Action
		INTO chInInstanceID, chOutInstanceID, chUserID, chOutAction
		FROM Signal AS S, SignalInstance AS SI
		WHERE S.SignalID = signal_id AND SI.SignalInstanceID = S.CheckedOutInstanceID;

		IF chOutInstanceID IS NULL THEN
			-- signal is not checked out, nothing to check in
			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = FALSE;
			os.action = 0;
			os.userID = 0;
			os.errCode = 1;			-- ERR_SIGNAL_IS_NOT_CHECKED_OUT
			RETURN NEXT os;

		ELSE
			IF chUserID = user_id OR userIsAdmin THEN
				-- signal checked out by current user, or user is admin

				IF chOutAction = 3 AND chInInstanceID IS NULL THEN
					-- action is DELETE, and signal has no checked in instance
					-- remove checked out instance and signal
					DELETE FROM CheckOut WHERE SignalID = signal_id;
					UPDATE Signal SET CheckedOutInstanceID = NULL WHERE SignalID = signal_id;
					DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;
					DELETE FROM Signal WHERE SignalID = signal_id;

					os.ID = signal_id;
					os.deleted = TRUE;
					os.checkedout = FALSE;
					os.action = 0;
					os.userID = 0;
					os.errCode = 0;				-- ERR_SIGNAL_OK
					RETURN NEXT os;

				ELSE
					UPDATE SignalInstance SET ChangesetID = newChangesetID
					WHERE SignalInstanceID = chOutInstanceID;

					--signalDeleted =

					UPDATE Signal
					SET
						CheckedInInstanceID = CheckedOutInstanceID,
						CheckedOutInstanceID = NULL,
						UserID = NULL,
						Deleted = (chOutAction = 3)
					WHERE
						SignalID = signal_id;

					DELETE FROM CheckOut WHERE SignalID = signal_id;

					os.ID = signal_id;
					os.deleted = FALSE;
					os.checkedout = FALSE;
					os.action = 0;
					os.userID = 0;
					os.errCode = 0;				-- ERR_SIGNAL_OK
					RETURN NEXT os;

				END IF;

				checkedInSignalCount = checkedInSignalCount + 1;

			ELSE
				-- signal checked out by another user
				os.ID = signal_id;
				os.deleted = FALSE;
				os.checkedout = TRUE;
				os.action = 0;
				os.userID = chUserID;
				os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
				RETURN NEXT os;

			END IF;
		END IF;
	END LOOP;

	IF checkedInSignalCount = 0 THEN
		-- no signals checked in, remove created changeset
		DELETE FROM Changeset WHERE ChangesetID = newChangesetID;
	END IF;
END;
$BODY$
  LANGUAGE plpgsql;


DROP FUNCTION checkout_signals(integer, integer[]);

CREATE OR REPLACE FUNCTION checkout_signals(user_id integer, signal_ids integer[])
  RETURNS SETOF objectstate AS
$BODY$
DECLARE
	alreadyCheckedOut integer;
	signal_id integer;
	chOutInstanceID integer;
	os objectstate;
	chOutUserID integer;
BEGIN
	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT UserID INTO chOutUserID FROM CheckOut WHERE SignalID = signal_id;

		IF chOutUserID IS NOT NULL THEN
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
					DeviceStrID )
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
					DeviceStrID
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
			os.action = 0;
			os.userID = user_id;
			os.errCode = 0;					-- ERR_SIGNAL_OK
			RETURN NEXT os;

		END IF;
	END LOOP;
END
$BODY$
  LANGUAGE plpgsql;
