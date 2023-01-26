-- Create $root$/Tests/HardwareTests
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$/Tests', 'HardwareTests', 'Update: Adding file $root$/Tests/HardwareTests', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Tests/HardwareTests', 1);
