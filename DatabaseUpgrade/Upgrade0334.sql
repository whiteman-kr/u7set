-- Create $root$/Tests
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$', 'Tests', 'Update: Adding file $root$/Tests', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Tests', 1);

SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$/Tests', 'SimTests', 'Update: Adding file $root$/Tests/SimTests', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Tests/SimTests', 1);
