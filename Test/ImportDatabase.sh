#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]
then
      echo "Database name is empty! Syntax: ImportDatabase.sh dababase_file.sql database_name"
      exit 1
fi

if [[ -f "$1" ]]; then
    echo "File $1 exists."
else
    echo "File $1 does not exist!"
    exit 1
fi

if psql -lqt --port 5433 | cut -d \| -f 1 | grep -qw $2; then
    echo Database $2 already exists, deleting it.
    psql -U postgres --port 5433 -c "DROP DATABASE $2;"
fi

echo Importing database $2.

psql -U postgres --port 5433 -c "CREATE DATABASE $2 OWNER u7;"
time psql $2 < $1
