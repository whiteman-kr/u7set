-- RPCT- 2056
-- Add system folder ETC
--

SELECT check_in(1, ARRAY(SELECT id FROM add_file(1, 'ETC', 0, '', '{}')), 'Upgrade: ETC system folder was added');