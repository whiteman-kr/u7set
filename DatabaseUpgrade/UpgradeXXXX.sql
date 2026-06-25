-- Create folder $root$/Schemas/Actuators
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$/Schemas', 'Actuators', 'Update: Adding file $root$/Schemas/Acturators', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Schemas/Actuators', 1);
