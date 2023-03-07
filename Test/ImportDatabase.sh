#!/bin/bash

if [ -z "$1" ]
then
      echo "Database name is empty! Syntax: ImportDatabase.sh database_name_without_dot_sql"
      exit 1
fi

if [[ -f "$1.sql" ]]; then
    echo "File $1.sql exists."
else
    echo "File $1.sql does not exist!"
    exit 1
fi

if psql -lqt | cut -d \| -f 1 | grep -qw $1; then
    echo Database $1 already exists, deleting it.
    psql -U postgres -c "DROP DATABASE $1;"
fi

echo Importing database $1.

psql -U postgres -c "CREATE DATABASE $1 OWNER u7;"
psql $1 < $1.sql
