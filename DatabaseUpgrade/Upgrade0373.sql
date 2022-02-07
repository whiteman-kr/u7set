DROP FUNCTION IF EXISTS public.get_signals_id_appsignalid;
DROP TYPE IF EXISTS public.signal_id_appsignalid;

--

CREATE TYPE public.signal_id_appsignalid AS
(
	id integer,
	signalgroupid integer,
	appsignalid text
);

CREATE OR REPLACE FUNCTION public.get_signals_id_appsignalid(
	user_id integer,
	with_deleted boolean)
	RETURNS SETOF signal_id_appsignalid
	LANGUAGE 'plpgsql'

	COST 100
	VOLATILE
	ROWS 1000
AS $BODY$
BEGIN
	RETURN QUERY
		SELECT SI.SignalID, S.SignalGroupID, SI.AppSignalID
		FROM
			SignalInstance AS SI,
			Signal AS S
		WHERE
			SI.SignalInstanceID IN
			(SELECT * FROM get_signals_actual_signalinstanceid(user_id, with_deleted)) AND
			SI.SignalID = S.SignalID
		ORDER BY
			SI.SignalID ASC;
END
$BODY$;

