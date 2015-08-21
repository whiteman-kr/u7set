-- Object state
--
CREATE TYPE objectstate AS (
	id integer,					-- ObjectID, cant be FileID, SignalID etc
	deleted boolean,			-- File or signal was deleted from the table, so such ID is not exists anymore
	checkedout boolean,			-- If true, then file is CheckedOut, else it is in CheckedIn state.
	action integer,				-- action
	userid integer,
	errcode integer
);
