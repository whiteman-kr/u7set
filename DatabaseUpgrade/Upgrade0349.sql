-- Create $root$/DiagSignalTypes
--
SELECT * FROM api.add_or_update_file('$(SessionKey)', '$root$', 'DiagSignalTypes', 'Update: Adding file $root$/DiagSignalTypes', '', '{}');
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/DiagSignalTypes', 1);

-- Set parent for schemas, $root$/Diagnostics -> $root$/Schemas/Diagnostics
--
UPDATE File
    SET ParentID = (SELECT FileID FROM File WHERE Name = 'Schemas' AND ParentID = 0)
	    WHERE Name = 'Diagnostics' AND ParentID = 0;

-- Set Folder attribute
--
SELECT * FROM api.set_file_attributes('$(SessionKey)', '$root$/Schemas/Diagnostics', 1);

