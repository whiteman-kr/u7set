CREATE TABLE systeminst (
    systeminstid serial PRIMARY KEY,
    systemid int,
    changesetid int REFERENCES changeset,
    state int,
    strid text,
    caption text,
    pos int );


CREATE TABLE caseinst (
    caseinstid serial PRIMARY KEY,
    caseid int,
    systemid int NOT NULL,
    changesetid int REFERENCES changeset,
    state int,
    strid text,
    caption text,
    pos int,
    place int );


CREATE TABLE subblockinst (
    subblockinstid serial PRIMARY KEY,
    subblockid int,
    caseid int,
    systemid int,
    changesetid int REFERENCES changeset,
    state int,
    strid text,
    caption text,
    pos int,
    place int );


CREATE TABLE blockinst (
    blockinstid serial PRIMARY KEY,
    blockid int,
    subblockid int,
    caseid int,
    systemid int,
    changesetid int REFERENCES changeset,
    state int,
    strid text,
    caption text,
    pos int,
    place int );

