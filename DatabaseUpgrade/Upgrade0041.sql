-- MVS		Monitor Visualization Schemas, RPCT-673
SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'MVS', 0, '', '{}')), 'Upgrade: MVS system folder was added');
