CREATE FUNCTION UndoFilePendingChanges(IN FileID integer, IN UserID integer) RETURNS boolean AS
$BODY$
    DELETE FROM FileInstance WHERE FileID=$1 AND ChangesetID IS NULL;
    DELETE FROM CheckOut WHERE FileID=$1 AND UserID=$2;

    DELETE FROM	File
        WHERE FileID=$1 AND 0 = (SELECT COUNT(FileID) FROM FileInstance WHERE FileId = $1);

    SELECT TRUE;
$BODY$
LANGUAGE sql;
