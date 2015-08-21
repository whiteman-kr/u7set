ALTER TABLE checkout
  ADD CONSTRAINT checkout_signalid_fkey FOREIGN KEY (signalid)
	  REFERENCES signal (signalid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION;


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
	actionDelete boolean;
	sGroupID integer;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	INSERT INTO Changeset (UserID, Comment, File) VALUES (user_id, checkin_comment, FALSE) RETURNING ChangesetID INTO newChangesetID;

	checkedInSignalCount = 0;

	FOREACH signal_id IN ARRAY signal_ids
	LOOP
		SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID, SI.Action, S.SignalGroupID
		INTO chInInstanceID, chOutInstanceID, chUserID, chOutAction, sGroupID
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

				IF chOutAction = 3 THEN
					actionDelete = TRUE;
				ELSE
					actionDelete = FALSE;
				END IF;

				IF actionDelete AND chInInstanceID IS NULL THEN
					-- action is DELETE, and signal has no checked in instance
					-- remove checked out instance and signal
					DELETE FROM CheckOut WHERE SignalID = signal_id;
					UPDATE Signal SET CheckedOutInstanceID = NULL WHERE SignalID = signal_id;
					DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;
					DELETE FROM Signal WHERE SignalID = signal_id;

					IF sGroupID <> 0 THEN
						DELETE FROM SignalGroup
						WHERE NOT EXISTS(SELECT * FROM Signal AS S WHERE SignalGroupID = sGroupID) AND SignalGroupID = sGroupID;
					END IF;

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
						Deleted = actionDelete
					WHERE
						SignalID = signal_id;

					DELETE FROM CheckOut WHERE SignalID = signal_id;

					os.ID = signal_id;
					os.deleted = actionDelete;
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
						DeviceStrID )
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
				os.action = 2;
				os.userID = user_id;
				os.errCode = 0;					-- ERR_SIGNAL_OK
				RETURN NEXT os;

			END IF;
		END IF;
	END LOOP;
END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION delete_signal(user_id integer, signal_id integer)
  RETURNS objectstate AS
$BODY$
DECLARE
	os objectstate;
	chInInstanceID integer;
	chOutInstanceID integer;
	chOutUserID integer;
	sGroupID integer;
BEGIN
	SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID, S.SignalGroupID
	INTO chInInstanceID, chOutInstanceID, chOutUserID, sGroupID
	FROM Signal AS S WHERE SignalID = signal_id;

	IF chInInstanceID IS NULL THEN
		-- no checked in instance
		-- delete checked out instance and signal
		DELETE FROM CheckOut WHERE SignalID = signal_id;
		UPDATE Signal SET CheckedOutInstanceID = NULL WHERE SignalID = signal_id;
		DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;
		DELETE FROM Signal WHERE SignalID = signal_id;

		IF sGroupID <> 0 THEN
			DELETE FROM SignalGroup
			WHERE NOT EXISTS(SELECT * FROM Signal AS S WHERE SignalGroupID = sGroupID) AND SignalGroupID = sGroupID;
		END IF;

		os.ID = signal_id;
		os.deleted = TRUE;
		os.checkedout = FALSE;
		os.action = 3;
		os.userID = 0;
		os.errCode = 0;					-- ERR_SIGNAL_OK
		RETURN os;
	ELSE
		-- has checked in instance

		IF chOutInstanceID IS NULL THEN
			-- signal is not checked out, - check out signal
			os = checkout_signals(user_id, ARRAY[signal_id]);
			IF os.errCode <> 0 THEN
				RETURN os;
			END IF;

			SELECT CheckedOutInstanceID, UserID INTO chOutInstanceID, chOutUserID FROM Signal WHERE SignalID = signal_id;
		END IF;

		-- signal is checked out

		IF chOutUserID = user_id THEN
			-- signal checked out by this user
			-- change checked out signal instance action to Delete (3)

			UPDATE SignalInstance
			SET Action = 3
			WHERE SignalInstanceID = (SELECT CheckedOutInstanceID FROM Signal WHERE SignalID = signal_id);

			os.ID = signal_id;
			os.deleted = TRUE;
			os.checkedout = TRUE;
			os.action = 3;
			os.userID = user_id;
			os.errCode = 0;					-- ERR_SIGNAL_OK
		ELSE
			-- signal is checked out by another user

			os.ID = signal_id;
			os.deleted = FALSE;
			os.checkedout = TRUE;
			os.action = 0;
			os.userID = chOutUserID;
			os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
		END IF;

		RETURN os;
	END IF;
END
$BODY$
  LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION undo_signal_changes(user_id integer, signal_id integer)
  RETURNS objectstate AS
$BODY$
DECLARE
	os objectstate;
	userIsAdmin boolean;
	chInInstanceID integer;
	chOutInstanceID integer;
	chOutUserID integer;
	sGroupID integer;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	SELECT S.CheckedInInstanceID, S.CheckedOutInstanceID, S.UserID, S.SignalGroupID
	INTO chInInstanceID, chOutInstanceID, chOutUserID, sGroupID
	FROM Signal AS S WHERE SignalID = signal_id;

	IF chOutInstanceID IS NULL THEN
		os.ID = signal_id;
		os.deleted = FALSE;
		os.checkedout = FALSE;
		os.action = 0;
		os.userID = 0;
		os.errCode = 1;					-- ERR_SIGNAL_IS_NOT_CHECKED_OUT
		RETURN os;
	END IF;

	IF chOutUserID = user_id OR userIsAdmin THEN
		DELETE FROM CheckOut WHERE SignalID = signal_id;
		UPDATE Signal SET CheckedOutInstanceID = NULL, UserID = NULL WHERE SignalID = signal_id;
		DELETE FROM SignalInstance WHERE SignalInstanceID = chOutInstanceID;

		os.deleted = FALSE;

		IF chInInstanceID IS NULL THEN
			DELETE FROM Signal WHERE SignalID = signal_id;

			os.deleted = TRUE;

			IF sGroupID <> 0 THEN
				DELETE FROM SignalGroup
				WHERE NOT EXISTS(SELECT * FROM Signal AS S WHERE SignalGroupID = sGroupID) AND SignalGroupID = sGroupID;
			END IF;
		END IF;

		os.ID = signal_id;
		os.checkedout = FALSE;
		os.action = 0;
		os.userID = 0;
		os.errCode = 0;					-- ERR_SIGNAL_OK
		RETURN os;

	ELSE
		os.ID = signal_id;
		os.deleted = FALSE;
		os.checkedout = TRUE;
		os.action = 0;
		os.userID = chOutUserID;
		os.errCode = 2;					-- ERR_SIGNAL_ALREADY_CHECKED_OUT
		RETURN os;
	END IF;
END
$BODY$
  LANGUAGE plpgsql;


DROP FUNCTION set_signal_workcopy(integer, signaldata);

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
			DeviceStrID = sd.DeviceStrID
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
  LANGUAGE plpgsql;

