CREATE TYPE public.signal_id_appsignalid AS
(
	id integer,
	appsignalid text
);

CREATE OR REPLACE FUNCTION public.get_signals_id_appsignalid(
	user_id integer,
	with_deleted boolean)
  RETURNS SETOF signal_id_appsignalid AS
$BODY$
DECLARE
	userIsAdmin boolean;
BEGIN
	-- select IDs and AppSignalIDs of signals
	-- that checked in and/or checked out by user_id
	-- Signal must have corresponding SignalInstance

	SELECT is_admin(user_id) INTO userIsAdmin;

	RETURN QUERY
		SELECT S.SignalID, SI.AppSignalID
		FROM
			Signal AS S,
			SignalInstance AS SI
		WHERE
			SI.SignalID = S.SignalID AND
			((S.CheckedInInstanceID IS NOT NULL) OR (S.CheckedOutInstanceID IS NOT NULL AND (S.UserID = user_id OR userIsAdmin))) AND
			(S.Deleted != TRUE OR with_deleted)
		ORDER BY
			S.SignalID ASC;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
