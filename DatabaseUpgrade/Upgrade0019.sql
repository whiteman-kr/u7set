-- Add System folders (files) for different types
-- AFBL		Application Functional Block Library
-- AL		Application Logic
-- HC		Hardware Configuration
-- HP		Hardware Preset
-- WVS		Workflow Visualization Schemas, Is not used anymore, kept for compatibility, TASK RPCT-673
-- DVS		Diagnostics Visualization Schemas

SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'AFBL', 0, 0, '')), 1, 'Upgrade: AFBL system folder was added');
SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'AL', 0, 0, '')), 1, 'Upgrade: AL system folder was added');
SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'HC', 0, 0, '')), 1, 'Upgrade: HC system folder was added');
SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'HP', 0, 0, '')), 1, 'Upgrade: HP system folder was added');

-- Is not used anymore, kept for compatibility, TASK RPCT-673
SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'WVS', 0, 0, '')), 1, 'Upgrade: WVS system folder was added');

SELECT CheckIn(ARRAY(SELECT * FROM AddFile(1, 'DVS', 0, 0, '')), 1, 'Upgrade: DVS system folder was added');
