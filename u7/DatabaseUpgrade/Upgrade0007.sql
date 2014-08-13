CREATE EXTENSION "uuid-ossp";

CREATE TABLE FileInstance (
    FileInstanceID uuid PRIMARY KEY DEFAULT uuid_generate_v1(),
    Sequence serial NOT NULL,
    Data bytea,
    Size integer NOT NULL,
    FileID integer NOT NULL REFERENCES File,
    ChangesetID integer REFERENCES Changeset,
    Created timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP );
