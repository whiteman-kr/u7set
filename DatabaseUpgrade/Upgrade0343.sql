CREATE OR REPLACE FUNCTION public.get_specific_signals_all_by_changeset_id(
    user_id integer,
	changeset_id integer)
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
		S.UserID,				-- signal checked out for user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	    SI.SignalID = S.SignalID AND
		SI.SignalInstanceID IN (
		    SELECT MAX(SI.signalInstanceID) AS signalInstance
			    FROM signal AS S, signalInstance AS SI
				    WHERE S.signalID = SI.signalID AND SI.changesetID IS NOT NULL AND SI.changesetID <= changeset_ID GROUP BY S.signalID
		)
	ORDER BY S.SignalID;
END
$BODY$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION public.get_specific_signals_all_by_date(user_id integer, changeset_date timestamp with time zone)
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
		S.UserID,				-- signal checked out for user with user_id
		S.Created,
		S.Deleted,
		SI.Created,				-- InstanceCreated timestamp with time zone,
		SI.Action				-- InstanceAction
	FROM Signal AS S, SignalInstance AS SI
	WHERE
	    SI.SignalID = S.SignalID AND
		SI.SignalInstanceID IN (
		    SELECT MAX(SI.signalInstanceID) AS signalInstance
			    FROM signal AS S, signalInstance AS SI, changeset AS CS
				    WHERE S.signalID = SI.signalID AND SI.changesetid = CS.changesetID
					    AND CS.time <= changeset_date GROUP BY S.signalID
		)
	ORDER BY S.SignalID;
END
$BODY$
LANGUAGE plpgsql;
