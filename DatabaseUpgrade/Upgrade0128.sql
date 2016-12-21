--
--  RPCT-1376, fix spell of administrator word in dbuser type
--
ALTER TYPE user_api.dbuser RENAME ATTRIBUTE admininstrator TO administrator;