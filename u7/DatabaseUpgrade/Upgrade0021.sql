CREATE OR REPLACE FUNCTION addsystem(user_id integer, file_data bytea)
  RETURNS int AS
$BODY$
DECLARE
	hc_fileid int;
	fileid_lenght int;
	new_filename text;
	new_fileid int;
BEGIN
	-- generate filename
	SELECT max(fileid) INTO new_filename FROM file;
	SELECT octet_length(new_filename) INTO fileid_lenght;

	FOR i IN 1..(10 - fileid_lenght) LOOP
		new_filename := '0' || new_filename;
	END LOOP;

	new_filename := 'device-' || new_filename || '.hcsystem';

	-- Get HC file fileId
	SELECT fileid INTO hc_fileid FROM
		(SELECT fileid FROM getfilelist(0, 'HC')) hct;

	-- add new file
	SELECT * INTO new_fileid FROM addfile(user_id, new_filename, hc_fileid, length(file_data), file_data);

	RETURN new_fileid;
END
$BODY$
LANGUAGE plpgsql;
