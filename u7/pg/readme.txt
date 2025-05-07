This folder is the default location for searching for `pg_dump` and `psql` utilities.

These tools are required for backup and restore functionality in the application.  
Ensure that the appropriate versions of `pg_dump` and `psql` are available in this directory  
or update the application's configuration if they are located elsewhere.

pg_dump: Used for creating backups of PostgreSQL databases.  
psql: Used for restoring and managing PostgreSQL databases.

If these tools are missing, please install PostgreSQL and copy the necessary executables into this folder.
