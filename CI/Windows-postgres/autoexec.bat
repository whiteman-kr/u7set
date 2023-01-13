@echo off
echo Waiting for PostgreSQL to start...
ping -n 15 127.0.0.1 > nul
echo Creating PostgreSQL users...
psql -U postgres -c "CREATE USER u7; ALTER USER u7 WITH PASSWORD 'P2ssw0rd'; ALTER ROLE u7 SUPERUSER CREATEDB LOGIN;" > nul
:end

