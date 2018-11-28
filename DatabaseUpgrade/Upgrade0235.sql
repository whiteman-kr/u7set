CREATE OR REPLACE FUNCTION api.get_file_list_tree(
    session_key text,
    parent_id integer,
    file_mask text,
    remove_deleted boolean)
RETURNS SETOF dbfileinfo AS
$BODY$
DECLARE
    result dbfileinfo[];
BEGIN    
	-- Check session_key and raise error if it is wrong
	--
	PERFORM user_api.check_session_key(session_key, TRUE);    

	-- Get all files list
	--
	RETURN QUERY 
	(
        WITH RECURSIVE files AS (
            SELECT * FROM api.get_file_list(session_key, parent_id, file_mask) WHERE (Deleted = FALSE) OR (Deleted = TRUE AND remove_deleted = FALSE)
                UNION ALL
            SELECT FL.* 
                FROM Files, api.get_file_list(session_key, files.FileID, file_mask) FL 
                WHERE ((FL.Deleted = FALSE) OR (FL.Deleted = TRUE AND remove_deleted = FALSE)) AND 
                        EXISTS (SELECT * FROM File WHERE File.ParentID = Files.FileID)
        )
        SELECT * FROM files
			UNION
		SELECT * FROM api.get_file_list(session_key, (SELECT ParentID FROM File WHERE FileID = parent_id), file_mask) WHERE FileID = parent_id
    );
END
$BODY$
LANGUAGE plpgsql;