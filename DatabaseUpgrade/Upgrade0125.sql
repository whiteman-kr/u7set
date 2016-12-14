-- RPCT-1345 -- Add file CONNECTION
SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'CONNECTIONS', 0, '', '{}')), 'Upgrade: CONNECTIONS system folder was added');
