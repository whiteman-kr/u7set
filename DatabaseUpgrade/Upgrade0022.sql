DROP FUNCTION addsystem(user_id integer, file_data bytea);

CREATE OR REPLACE FUNCTION add_device(user_id integer, file_data bytea, parent_id integer, file_extension text)
RETURNS int AS
$BODY$
DECLARE
	hc_fileid int;
	fileid_lenght int;
	new_filename text;
	new_fileid int;
BEGIN
	-- TO DO: Check user right here

	-- generate filename
	SELECT * INTO new_filename FROM uuid_generate_v4();
	SELECT octet_length(new_filename) INTO fileid_lenght;

	new_filename := 'device-' || new_filename || file_extension;	-- smthng like: device-5be363ac-3c02-11e4-9de8-3f84f459cb27.hsystem

	-- add new file
	SELECT * INTO new_fileid FROM addfile(user_id, new_filename, parent_id, length(file_data), file_data);

	RETURN new_fileid;
END
$BODY$
LANGUAGE plpgsql;
