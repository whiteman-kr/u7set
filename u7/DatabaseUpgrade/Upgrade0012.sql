ALTER TABLE checkout ADD COLUMN systeminstid int UNIQUE REFERENCES SystemInst;

ALTER TABLE checkout ADD COLUMN caseinstid int UNIQUE REFERENCES CaseInst;

ALTER TABLE checkout ADD COLUMN subblockinstid int UNIQUE REFERENCES SubblockInst;

ALTER TABLE checkout ADD COLUMN blockinstid int UNIQUE REFERENCES BlockInst;
