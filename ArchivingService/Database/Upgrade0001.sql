CREATE SEQUENCE archid_seq INCREMENT BY 1 CACHE 1;


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
					      locTime bigint,
					      val double precision,
					      flags integer,
					      isAnalogSignal boolean,
					      writeInShortTermArchiveOnly boolean)
  RETURNS bigint AS
$BODY$
DECLARE
        hashHex text;
	shortTermTable text;
	longTermTable text;
	archid bigint;
BEGIN
        hashHex = int64hex(signalHash);

        IF isAnalogSignal = TRUE THEN

                shortTermTable = concat('st_', hashHex);

                -- write signal state in short term table

                EXECUTE format('INSERT INTO %I (plantTime, sysTime, locTime, val, flags)'
		        'VALUES (%L,%L,%L,%L,%L) RETURNING %I',
			shortTermTable, plantTime, sysTime, locTime, val, flags, 'archid')
		INTO archid;

                -- test if set smoothAperture flag only

                IF writeInShortTermArchiveOnly = TRUE THEN
		        RETURN archid;
		END IF;

                -- write signal state in long ter table

                longTermTable = concat('lt_', hashHex);

                EXECUTE format('INSERT INTO %I (archid, plantTime, sysTime, locTime, val, flags)'
		        'VALUES (%L,%L,%L,%L,%L,%L)',
			longTermTable, archid, plantTime, sysTime, locTime, val, flags);

                RETURN archid;
	ELSE
	        longTermTable = concat('lt_', hashHex);

                archid = nextval('archid_seq'::regclass);

                EXECUTE format('INSERT INTO %I (archid, plantTime, sysTime, locTime, val, flags)'
		        'VALUES (%L,%L,%L,%L,%L,%L)',
			longTermTable, archid, plantTime, sysTime, locTime, val, flags);

                RETURN archid;
	END IF;
END
$BODY$
 LANGUAGE plpgsql;
