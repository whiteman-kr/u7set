INSERT INTO file (fileid, name) VALUES (0, '$root$');

ALTER TABLE file
	ADD COLUMN parentid integer DEFAULT 0;

UPDATE file
	SET parentid=NULL WHERE fileId=0;

ALTER TABLE file
	ADD CONSTRAINT file_parentid_fkey FOREIGN KEY (parentid) REFERENCES file (fileid);

ALTER TABLE file
	ADD COLUMN deleted boolean DEFAULT false;

ALTER TABLE file
	DROP CONSTRAINT file_name_key;

ALTER TABLE fileinstance
	ADD COLUMN action integer DEFAULT 1;

ALTER TABLE fileinstance
	ADD COLUMN details JSONB DEFAULT '{}';

