------------------------------------------------------------------------------
--
-- Create table: Build
-- Create stored procedures: build_start, build_finish
--
------------------------------------------------------------------------------


CREATE TABLE build
(
  buildid serial NOT NULL,
  userid integer,
  workstation text NOT NULL,
  release boolean NOT NULL,
  starttime time with time zone NOT NULL DEFAULT now(),
  errors integer NOT NULL DEFAULT 0,
  warnings integer NOT NULL DEFAULT 0,
  changesetid integer,
  builddata bytea,
  buildlog text,
  finishtime time with time zone,
  fileid integer,
  CONSTRAINT build_pkey PRIMARY KEY (buildid),
  CONSTRAINT build_changesetid_fkey FOREIGN KEY (changesetid)
	  REFERENCES changeset (changesetid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT build_fileid_fkey FOREIGN KEY (fileid)
	  REFERENCES file (fileid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT build_userid_fkey FOREIGN KEY (userid)
	  REFERENCES Users (UserID) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);


CREATE OR REPLACE FUNCTION build_start(user_id integer, build_workstation text, release_build boolean, build_changesetid integer)
  RETURNS integer AS
$BODY$
DECLARE
	build_id integer;
BEGIN
	IF build_changesetid <> 0 THEN
		INSERT INTO build (userid, workstation, release, changesetid)
			VALUES (user_id, build_workstation, release_build, build_changesetid) RETURNING buildid INTO build_id;
	ELSE
		INSERT INTO build (userid, workstation, release)
			VALUES (user_id, build_workstation, release_build) RETURNING buildid INTO build_id;
	END IF;

	RETURN build_id;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;


CREATE OR REPLACE FUNCTION build_finish(build_id integer, err integer, wrn integer, build_log text) RETURNS void AS
$BODY$
BEGIN
	UPDATE build SET finishtime = now(), errors = err, warnings = wrn, buildlog = build_log WHERE buildid = build_id;
END
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
