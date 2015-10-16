-- Delete DataFormat table and remove corresponding constraints and stored procedures

ALTER TABLE signalinstance DROP CONSTRAINT dataformat_fkey;

UPDATE signalinstance SET dataformatid = dataformatid-1;

DROP TABLE dataformat;

DROP FUNCTION get_data_formats();


CREATE OR REPLACE FUNCTION delete_signal_by_device_str_id(user_id integer, signal_device_str_id text)
RETURNS objectstate AS
$BODY$
DECLARE
	os objectstate;
	result record;
BEGIN
	FOR result IN
		SELECT S.SignalID FROM Signal AS S, SignalInstance AS SI
		WHERE S.SignalID = SI.SignalID AND S.Deleted = FALSE AND SI.DeviceStrID = signal_device_str_id
	LOOP

		os = delete_signal(user_id, result.SignalID);

	END LOOP;

	return os;
END
$BODY$
  LANGUAGE plpgsql VOLATILE;
