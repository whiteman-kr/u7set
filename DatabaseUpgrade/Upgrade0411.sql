-- Create folder $root$/Schemas/VDU
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$/Schemas', 'VDU', 'Update: Adding file $root$/Schemas/VDU', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Schemas/VDU', 1);
