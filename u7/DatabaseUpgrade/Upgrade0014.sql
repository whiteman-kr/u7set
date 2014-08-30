CREATE OR REPLACE FUNCTION addfile(userid integer, filename text, filesize integer, filedata bytea)
RETURNS integer AS
$BODY$

INSERT INTO File (Name)
	VALUES (filename) RETURNING FileID;

INSERT INTO CheckOut (UserID, FileID)
	VALUES (userid, (SELECT FileID FROM File WHERE Name=filename));

INSERT INTO FileInstance (FileID, Size, data)
	VALUES ((SELECT FileID FROM File WHERE Name = filename), filesize, filedata) RETURNING FileID;

$BODY$
LANGUAGE sql;
