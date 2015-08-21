-- Delete DataFormat table and remove corresponding constraints and stored procedures

ALTER TABLE signalinstance DROP CONSTRAINT dataformat_fkey;

UPDATE signalinstance SET dataformatid = dataformatid-1;

DROP TABLE dataformat;

DROP FUNCTION get_data_formats();
