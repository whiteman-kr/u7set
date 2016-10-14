--
-- Add function is_signal_with_equipmentid_exists(user_id integer,  equipment_id text)
--

CREATE OR REPLACE FUNCTION is_signal_with_equipmentid_exists(
    user_id integer,
    equipment_id text)
  RETURNS boolean AS
$BODY$
DECLARE
        signalCount integer;
	userIsAdmin boolean;
	result boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0 THEN
	        RETURN FALSE;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.EquipmentID = equipment_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NOT NULL AND S.CheckedOutInstanceID = SI.SignalInstanceID AND (S.UserID = user_id OR userIsAdmin = TRUE));

        IF signalCount > 0 THEN
	        RETURN TRUE;
	END IF;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.EquipmentID = equipment_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NULL AND S.CheckedInInstanceID = SI.SignalInstanceID);

        RETURN signalCount > 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;


--
-- Add function is_signal_with_appsignalid_exists(user_id integer,  appsignal_id text)
--

CREATE OR REPLACE FUNCTION public.is_signal_with_appsignalid_exists(
    user_id integer,
    appsignal_id text)
  RETURNS boolean AS
$BODY$
DECLARE
        signalCount integer;
	userIsAdmin boolean;
	result boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0 THEN
	        RETURN FALSE;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.AppSignalID = appsignal_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NOT NULL AND S.CheckedOutInstanceID = SI.SignalInstanceID AND (S.UserID = user_id OR userIsAdmin = TRUE));

        IF signalCount > 0 THEN
	        RETURN TRUE;
	END IF;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.AppSignalID = appsignal_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NULL AND S.CheckedInInstanceID = SI.SignalInstanceID);

        RETURN signalCount > 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;


--
-- Add function is_signal_with_customappsignalid_exists(user_id integer,  customappsignal_id text)
--

CREATE OR REPLACE FUNCTION public.is_signal_with_customappsignalid_exists(
    user_id integer,
    customappsignal_id text)
  RETURNS boolean AS
$BODY$
DECLARE
        signalCount integer;
	userIsAdmin boolean;
	result boolean;
BEGIN
        IF (SELECT COUNT(*) FROM users WHERE userid = user_id) = 0 THEN
	        RETURN FALSE;
	END IF;

        SELECT is_admin(user_id) INTO userIsAdmin;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.CustomAppSignalID = customappsignal_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NOT NULL AND S.CheckedOutInstanceID = SI.SignalInstanceID AND (S.UserID = user_id OR userIsAdmin = TRUE));

        IF signalCount > 0 THEN
	        RETURN TRUE;
	END IF;

        SELECT
	        COUNT(*) INTO signalCount
	FROM
	        Signal AS S, SignalInstance AS SI
	WHERE
	        S.SignalID = SI.SignalID AND
		SI.CustomAppSignalID = customappsignal_id AND
		S.Deleted != TRUE AND
		(S.CheckedOutInstanceID IS NULL AND S.CheckedInInstanceID = SI.SignalInstanceID);

        RETURN signalCount > 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
