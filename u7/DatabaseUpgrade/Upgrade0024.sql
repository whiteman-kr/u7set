-- Add CheckedInInstanceID, CheckedOutInstanceID to table File
ALTER TABLE file
	ADD COLUMN checkedininstanceid uuid DEFAULT NULL REFERENCES FileInstance(FileInstanceID);

ALTER TABLE file
	ADD COLUMN checkedoutinstanceid uuid DEFAULT NULL REFERENCES FileInstance(FileInstanceID);


-- Update added columns to the correct state
UPDATE File SET CheckedInInstanceID =
	(SELECT
		FileInstanceID
	FROM
		FileInstance
	WHERE
		FileInstance.FileID = File.FileID AND
		FileInstance.ChangesetID IS NOT NULL AND
		FileInstance.Sequence =
			(SELECT max(Sequence) FROM FileInstance WHERE FileInstance.FileID=File.FileID AND FileInstance.ChangesetID IS NOT NULL)
	);

UPDATE File SET CheckedOutInstanceID =
	(SELECT
		FileInstanceID
	FROM
		FileInstance
	WHERE
		FileInstance.FileID = File.FileID AND
		FileInstance.ChangesetID IS NULL
	);
