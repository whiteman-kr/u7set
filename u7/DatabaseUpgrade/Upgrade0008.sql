CREATE TABLE CheckOut (
    CheckOutID serial PRIMARY KEY,
    Time timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UserID integer NOT NULL REFERENCES Users,
    FileID integer REFERENCES File,
    SignalID integer );
