CREATE TABLE Changeset (
    ChangesetID SERIAL PRIMARY KEY,
    Time timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
	UserID integer REFERENCES Users,
    Comment text NOT NULL );