#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
then
      echo "Database name is empty! Syntax: ImportDatabase.sh dababase_file.sql database_name postgresql_port"
      exit 1
fi

if [[ -f "$1" ]]; then
    echo "File $1 exists."
else
    echo "File $1 does not exist!"
    exit 1
fi

if psql -lqt --port $3 | cut -d \| -f 1 | grep -qw $2; then
    echo Database $2 already exists, deleting it.
    psql -U postgres --port $3 -c "DROP DATABASE $2;"
fi

echo Importing database $2.

psql -U postgres --port $3 -c "CREATE DATABASE $2 OWNER u7;"
time psql --port $3 $2 < $1
