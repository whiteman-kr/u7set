-------------------------------
-- Create test_api
-------------------------------

CREATE SCHEMA test_api;

-------------------------------
-- Create new function for
-- checking files md5 sum
-------------------------------

CREATE OR REPLACE FUNCTION test_api.check_md5()
  RETURNS TABLE(corrupted_fileInstanceId uuid) AS
$BODY$
BEGIN
	RETURN QUERY
		SELECT fileInstanceId FROM fileInstance WHERE md5(data) != md5;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------
-- Create new function for
-- checking files validation
-------------------------------

CREATE OR REPLACE FUNCTION test_api.validate_files()
	RETURNS void AS
$BODY$
DECLARE
	corruptedRow RECORD;
BEGIN
	-- Get all rows form file and fileInstance
	-- where checkedOutInstanceId not empty or 
	-- changetId is empty (checkedOut files)

	FOR corruptedRow IN (SELECT FI.changesetId, FI.fileInstanceId, F.checkedOutInstanceId, F.fileId
			FROM File F, fileInstance FI 
			WHERE F.fileId = FI.fileId AND (checkedOutInstanceId IS NOT NULL OR FI.changesetId IS NULL))
	LOOP
	
		IF (corruptedRow.changesetId IS NULL) THEN
			RAISE NOTICE 'FileInstance %: ChangesetId empty', corruptedRow.fileInstanceId;
		END IF;
		
		IF (corruptedRow.checkedOutInstanceId IS NOT NULL) THEN
			RAISE NOTICE 'FileId %: CheckedOutInstanceId is not empty', corruptedRow.fileId;
		END IF;

	END LOOP;
END
$BODY$
LANGUAGE plpgsql;

-------------------------------
-- Create new function for
-- checking signals validation
-------------------------------

CREATE OR REPLACE FUNCTION test_api.validate_signals()
	RETURNS void AS
$BODY$
DECLARE
	corruptedRow RECORD;
BEGIN

	-- Get all rows form signal and signalInstance
	-- where checkedOutInstanceId not empty or 
	-- changetId is empty (checkedOut signals)

	FOR corruptedRow IN (SELECT SI.changesetId, SI.signalInstanceId, S.checkedOutInstanceId, S.signalId
			FROM Signal S, signalInstance SI 
			WHERE S.signalId = SI.SignalId AND (checkedOutInstanceId IS NOT NULL OR SI.changesetId IS NULL))
	LOOP
	
		IF (corruptedRow.changesetId IS NULL) THEN
			RAISE NOTICE 'SignalInstance %: ChangesetId is empty', corruptedRow.signalInstanceId;
		END IF;
		
		IF (corruptedRow.checkedOutInstanceId IS NOT NULL) THEN
			RAISE NOTICE 'SignalId %: CheckedOutInstanceId is not empty', corruptedRow.signalId;
		END IF;

	END LOOP;
END
$BODY$
LANGUAGE plpgsql;