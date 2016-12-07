--
-- RPCT-1311: Bug fix in get_latest_signals_by_appsignalids() stored procedure
--

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
		SI.TuningDefaultValue
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
ALTER FUNCTION public.get_latest_signals_by_appsignalids(integer, text[])
  OWNER TO u7;
