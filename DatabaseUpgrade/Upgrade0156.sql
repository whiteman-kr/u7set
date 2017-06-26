--
-- Add file BUSTYPES
--
SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'BUSTYPES', 0, '', '{}')), 'Upgrade: BUSTYPES system folder was added');
