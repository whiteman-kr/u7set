-- RPCT-1273: Implement stored procedure get_signal_history()

CREATE OR REPLACE FUNCTION get_signal_history(
    user_id integer,
    signal_id integer)
  RETURNS SETOF dbchangeset AS
$BODY$
BEGIN
    IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
        THEN
	        RETURN;
	END IF;

        RETURN QUERY
	        SELECT DISTINCT
		        Changeset.ChangesetID AS ChangesetID,
			Changeset.UserID AS UserID,
			Users.Username AS Username,
			Changeset.Time AS Time,
			Changeset.Comment AS Comment,
			SignalInstance.Action AS Action
		FROM
		        SignalInstance, Changeset, Users
		WHERE
		        SignalInstance.SignalID = signal_id AND
			SignalInstance.ChangesetID =  Changeset.ChangesetID AND
			Changeset.UserID = Users.UserID
		ORDER BY
		        Changeset.ChangesetID DESC;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;



-- RPCT-1218: Implement stored procedure get_specific()

CREATE OR REPLACE FUNCTION get_specific_signal(
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
			SI.TuningDefaultValue
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
LANGUAGE plpgsql;
