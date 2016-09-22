-- UFBL		User Functional Block Library (schemas), RPCT-1062
--
SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'UFBL', 0, '', '{}')), 'Upgrade: UFBL system folder was added');
