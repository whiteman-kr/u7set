--
-- RPCT-1176 
--

--
-- Create table log
--
CREATE TABLE log
(	
	logid		serial 	PRIMARY KEY,
	time		timestamp with time zone DEFAULT now(),
	userid		integer	REFERENCES users,
	host		text,
	hostip		inet DEFAULT inet_client_addr(),	
	processid	bigint,
	pgbackendpid	int DEFAULT pg_backend_pid(),
	text		text	
);


--
-- Create function add_log_record(user_id INTEGER, text TEXT)
--
CREATE OR REPLACE FUNCTION add_log_record(user_id INTEGER, text TEXT)
RETURNS VOID AS $$
BEGIN
	INSERT INTO Log(UserID, Text) VALUES (user_id, text);
END;
$$ LANGUAGE plpgsql;


--
-- Create function add_log_record(user_id INTEGER, host TEXT, text TEXT)
--
CREATE OR REPLACE FUNCTION add_log_record(user_id INTEGER, host TEXT, text TEXT)
RETURNS VOID AS $$
BEGIN
	INSERT INTO Log(UserID, Host, Text) VALUES (user_id, host, text);
END;
$$ LANGUAGE plpgsql;


--
-- Create function add_log_record(user_id INTEGER, host TEXT, text TEXT)
--
CREATE OR REPLACE FUNCTION add_log_record(user_id INTEGER, host TEXT, process_id INTEGER, text TEXT)
RETURNS VOID AS $$
BEGIN
	INSERT INTO Log(UserID, Host, ProcessID, Text) VALUES (user_id, host, process_id, text);
END;
$$ LANGUAGE plpgsql;


