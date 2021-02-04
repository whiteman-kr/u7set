CREATE OR REPLACE FUNCTION public.get_latest_signals_all_with_user_id(
	user_id integer)
    RETURNS SETOF signaldata 
    LANGUAGE 'plpgsql'

    COST 100
    VOLATILE 
    ROWS 1000
AS $BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
	IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0
	THEN
		RETURN;
	END IF;

	SELECT is_admin(user_id) INTO userIsAdmin;

	RETURN QUERY 
	
	SELECT
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.EquipmentID,

		S.Type,
		SI.InOutType,

		SI.SpecPropStruct,
		SI.SpecPropValues,
		SI.ProtoData,

		S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
		CS.UserID,				-- signal checked in by user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI, Changeset AS CS
	WHERE
		SI.SignalID = S.SignalID AND
		S.Deleted != TRUE AND
		SI.ChangesetID = CS.ChangesetID AND
		SI.SignalInstanceID IN (

			SELECT
				SG.CheckedInInstanceID
			FROM
				Signal AS SG
			WHERE
				(SG.CheckedOutInstanceID IS NULL OR
				(SG.UserID <> user_id AND userIsAdmin = FALSE))
			)

	UNION ALL
	
	SELECT
		SI.AppSignalID,
		SI.CustomAppSignalID,
		SI.EquipmentID,

		S.Type,
		SI.InOutType,

		SI.SpecPropStruct,
		SI.SpecPropValues,
		SI.ProtoData,

		S.SignalID,
		S.SignalGroupID,
		SI.SignalInstanceID,
		SI.ChangesetID,
		(S.CheckedOutInstanceID IS NOT NULL),	-- CheckedOut
		S.UserID,				-- signal checked out by user with user_id
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
				SG.CheckedOutInstanceID
			FROM
				Signal AS SG
			WHERE
				(SG.CheckedOutInstanceID IS NOT NULL AND
				(SG.UserID = user_id OR userIsAdmin = TRUE))
			)
	 
	 ORDER BY SignalID ASC;

END
$BODY$;
