CREATE OR REPLACE FUNCTION get_latest_signals(user_id integer, signal_ids integer[])
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
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
		SI.SignalID = S.SignalID AND
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
  COST 100;



CREATE OR REPLACE FUNCTION get_latest_signals_all(user_id integer)
  RETURNS SETOF signaldata AS
$BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
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
  COST 100;



CREATE OR REPLACE FUNCTION get_signal_count(user_id integer)
  RETURNS integer AS
$BODY$
DECLARE
	userIsAdmin boolean;
	signal_count integer;
BEGIN
	SELECT is_admin(user_id) INTO userIsAdmin;

	SELECT
		COUNT(*)
	FROM
		Signal AS SG
	INTO
		signal_count
	WHERE
		(SG.CheckedOutInstanceID IS NULL)

		OR

		(SG.UserID = user_id OR userIsAdmin = TRUE);

	RETURN signal_count;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

