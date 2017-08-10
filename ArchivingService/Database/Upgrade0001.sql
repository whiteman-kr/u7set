CREATE SEQUENCE archid_seq INCREMENT BY 1 CACHE 1;

CREATE TYPE appsignalstate AS
   (signalHash bigint,
   plantTime bigint,
   sysTime bigint,
   val double precision,
   flags integer);


CREATE TABLE timemarks
(
  archid bigint NOT NULL DEFAULT nextval('archid_seq'::regclass),
  planttime bigint,
  systime bigint,
  servertime bigint,
  CONSTRAINT timemarks_pkey PRIMARY KEY (archid)
)
WITH (
  OIDS=FALSE
);

CREATE INDEX planttime_idx ON timemarks (planttime ASC);
CREATE INDEX systime_idx ON timemarks (systime ASC);
CREATE INDEX servertime_idx ON timemarks (servertime ASC);


CREATE OR REPLACE FUNCTION int64hex(val bigint)
  RETURNS text AS
$BODY$
DECLARE
        hexStr text;

        hexChars text[] := ARRAY[ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' ];
BEGIN

        FOR i IN 1..16 LOOP

                hexStr := concat(hexChars[(val & 15) + 1], hexStr);

                val = val >> 4;

        END LOOP;

        RETURN hexStr;
END
$BODY$
 LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION saveAppSignalState(signalHash bigint,
                                              plantTime bigint,
					      sysTime bigint,
					      val double precision,
					      flags integer)
  RETURNS bigint AS
$BODY$
DECLARE
        tableName text;
	archid bigint;
BEGIN
        tableName = concat('z_', int64hex(signalHash));

        EXECUTE format('INSERT INTO %I (plantTime, sysTime, val, flags)'
		        'VALUES (%L,%L,%L,%L,%L) RETURNING %I',
			tableName, plantTime, sysTime, val, flags, 'archid')
	INTO archid;

        RETURN archid;
END
$BODY$
 LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION public.saveappsignalstatesarray(appsignalstates appsignalstate[])
  RETURNS bigint AS
$BODY$
DECLARE
        tableName text;
	archid bigint;
	state AppSignalState;
BEGIN
        FOREACH state IN ARRAY appSignalStates LOOP

                tableName = concat('z_', int64hex(state.signalHash));

                EXECUTE format('INSERT INTO %I (plantTime, sysTime, val, flags)'
		        'VALUES (%L,%L,%L,%L) RETURNING %I',
			tableName, state.plantTime, state.sysTime, state.val, state.flags, 'archid')
		INTO archid;
	END LOOP;

        RETURN archid;
END
$BODY$
  LANGUAGE plpgsql;
