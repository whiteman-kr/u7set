-- Create table Unit

CREATE TABLE unit
(
  unitid serial NOT NULL,
  unit_en character varying(16) NOT NULL DEFAULT ''::character varying,
  unit_ru character varying(16) NOT NULL DEFAULT ''::character varying,
  CONSTRAINT unit_pkey PRIMARY KEY (unitid)
)
WITH (
  OIDS=FALSE
);

-- Initialize table Unit

INSERT INTO Unit (Unit_en, Unit_ru) VALUES
('', ''),

('mm', 'мм'),
('cm', 'см'),
('m', 'м'),

('°C', '°C'),
('°F', '°F'),

('Hz', 'Гц'),
('KHz', 'КГц'),
('MHz', 'МГц'),

('µV', 'мкВ'),
('mV', 'мВ'),
('V', 'В'),
('KV', 'КВ'),

('µA', 'мкА'),
('mA', 'мА'),
('A', 'А'),

('W', 'Вт'),
('kW', 'кВт'),
('MW', 'МВт'),

('Ohm', 'Ом'),
('KOhm', 'КОм'),

('Pa', 'Па'),
('KPa', 'КПа'),
('MPa', 'МПа'),

('cm/s2', 'см/с2'),
('m/s2', 'м/с2'),

('ms', 'мс'),
('s', 'с'),
('min', 'мин'),
('h', 'ч'),

('%', '%'),

('kgF', 'кгс'),
('kgF/cm2', 'кгс/см2'),


('t/h', 'т/ч'),
('rpm', 'об/мин'),

('m3/h', 'м3/ч');

-- Create table DataFormat

CREATE TABLE dataformat
(
  dataformatid serial NOT NULL,
  name character varying(32) NOT NULL,
  CONSTRAINT dataformat_pkey PRIMARY KEY (dataformatid)
)
WITH (
  OIDS=FALSE
);

-- Initialize table DataFormat

INSERT INTO DataFormat (Name) VALUES
('Binary LE unsigned'),
('Binary LE signed'),
('Binary BE unsigned'),
('Binary BE signed');

-- Create table SignalGroup

CREATE TABLE signalgroup
(
  signalgroupid serial NOT NULL,
  CONSTRAINT signalgroup_pkey PRIMARY KEY (signalgroupid)
)
WITH (
  OIDS=FALSE
);

-- Initialize table SignalGroup
-- SiganlGroupID = 0 - default group for all single-channel signals

INSERT INTO SignalGroup (SignalGroupID) VALUES (0);

-- Create table Signal

CREATE TABLE signal
(
  signalid serial NOT NULL,
  signalgroupid integer NOT NULL,
  channel integer NOT NULL,
  type integer NOT NULL,
  created timestamp with time zone NOT NULL DEFAULT now(),
  deleted boolean NOT NULL DEFAULT false,
  CONSTRAINT signal_pkey PRIMARY KEY (signalid),
  CONSTRAINT signalgroup_fkey FOREIGN KEY (signalgroupid)
	  REFERENCES signalgroup (signalgroupid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);

-- Create table SignalInstance

CREATE TABLE signalinstance
(
  signalinstanceid serial NOT NULL,
  signalid integer NOT NULL,
  changesetid integer,
  created timestamp with time zone NOT NULL DEFAULT now(),
  action integer NOT NULL DEFAULT 1,
  strid text NOT NULL,
  extstrid text NOT NULL,
  name text,
  dataformatid integer NOT NULL DEFAULT 1,
  datasize integer NOT NULL DEFAULT 1,
  lowadc integer NOT NULL DEFAULT 0,
  highadc integer NOT NULL DEFAULT 65535,
  lowlimit double precision NOT NULL DEFAULT 0,
  highlimit double precision NOT NULL DEFAULT 0,
  unitid integer NOT NULL DEFAULT 1,
  adjustment double precision NOT NULL DEFAULT 0,
  droplimit double precision NOT NULL DEFAULT 0,
  excesslimit double precision NOT NULL DEFAULT 0,
  unbalancelimit double precision NOT NULL DEFAULT 0,
  inputlowlimit double precision NOT NULL DEFAULT 0,
  inputhighlimit double precision NOT NULL DEFAULT 0,
  inputunitid integer NOT NULL DEFAULT 1,
  inputsensorid integer,
  outputlowlimit double precision NOT NULL DEFAULT 0,
  outputhighlimit double precision NOT NULL DEFAULT 0,
  outputunitid integer NOT NULL DEFAULT 1,
  outputsensorid integer,
  acquire boolean NOT NULL DEFAULT true,
  calculated boolean NOT NULL DEFAULT false,
  normalstate integer NOT NULL DEFAULT 0,
  decimalplaces integer NOT NULL DEFAULT 2,
  aperture double precision NOT NULL DEFAULT 0,
  inouttype integer NOT NULL DEFAULT 2,
  deviceid integer,
  inoutno integer,

  CONSTRAINT signalinstance_pkey PRIMARY KEY (signalinstanceid),
  CONSTRAINT changeset_fkey FOREIGN KEY (changesetid)
	  REFERENCES changeset (changesetid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT dataformat_fkey FOREIGN KEY (dataformatid)
	  REFERENCES dataformat (dataformatid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT inputunit_fkey FOREIGN KEY (inputunitid)
	  REFERENCES unit (unitid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT outputunit_fkey FOREIGN KEY (outputunitid)
	  REFERENCES unit (unitid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT signal_fkey FOREIGN KEY (signalid)
	  REFERENCES signal (signalid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT unit_fkey FOREIGN KEY (unitid)
	  REFERENCES unit (unitid) MATCH SIMPLE
	  ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
