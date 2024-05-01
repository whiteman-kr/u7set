-- Create folder $root$/AppSignalLists
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$', 'AppSignalLists', 'Update: Adding file $root$/AppSignalLists', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/AppSignalLists', 1);
