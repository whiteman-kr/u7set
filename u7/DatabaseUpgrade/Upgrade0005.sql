CREATE TABLE File (
    FileID serial PRIMARY KEY,
    Name text NOT NULL DEFAULT '',
    Created timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (Name) );
