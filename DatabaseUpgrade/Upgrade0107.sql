-- RPCT-1131
-- Add column host to table Version, so it's possible to know who upgraded the porject database
--
ALTER TABLE version ADD COLUMN host text NOT NULL DEFAULT 'Unknown';