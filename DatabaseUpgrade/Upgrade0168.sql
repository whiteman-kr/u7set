-- Add column unit into signalInstance table

ALTER TABLE public.signalinstance ADD COLUMN unit text;

-- Set values of column unit from unit table

UPDATE signalInstance AS si SET (unit) = (SELECT unit_en FROM unit WHERE unitid = si.unitid);

-- Drop constraints to unit table

ALTER TABLE public.signalinstance DROP CONSTRAINT inputunit_fkey;
ALTER TABLE public.signalinstance DROP CONSTRAINT outputunit_fkey;
ALTER TABLE public.signalinstance DROP CONSTRAINT unit_fkey;

-- Make other changes in signalInstance table

ALTER TABLE public.signalinstance DROP COLUMN unitid;
ALTER TABLE public.signalInstance RENAME COLUMN dataformatid TO analogsignalformat;
ALTER TABLE public.signalInstance DROP COLUMN adjustment;
ALTER TABLE public.signalInstance DROP COLUMN unbalancelimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputlowlimit TO electriclowlimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputhighlimit TO electrichighlimit;
ALTER TABLE public.signalInstance RENAME COLUMN inputunitid TO electricunit;
ALTER TABLE public.signalInstance RENAME COLUMN inputsensorid TO sensortype;
ALTER TABLE public.signalInstance DROP COLUMN outputlowlimit;
ALTER TABLE public.signalInstance DROP COLUMN outputhighlimit;
ALTER TABLE public.signalInstance DROP COLUMN outputunitid;
ALTER TABLE public.signalInstance DROP COLUMN outputsensorid;
ALTER TABLE public.signalInstance DROP COLUMN calculated;
ALTER TABLE public.signalInstance DROP COLUMN normalstate;
ALTER TABLE public.signalInstance RENAME COLUMN aperture TO coarseaperture;
ALTER TABLE public.signalinstance ADD COLUMN fineaperture double precision NOT NULL DEFAULT 0.5;
ALTER TABLE public.signalInstance RENAME COLUMN outputrangemode TO outputmode;

-- Drop unit table

DROP TABLE public.unit;

-- Drop all stored procedures dependent from SignalData type

/*
DROP FUNCTION get_latest_signal(integer, integer);
DROP FUNCTION set_signal_workcopy(integer, signaldata);
DROP FUNCTION get_latest_signals(integer, integer[]);
DROP FUNCTION get_latest_signals_all(integer);
DROP FUNCTION checkout_signals(integer, integer[]);
DROP FUNCTION get_specific_signal(integer,integer,integer);
DROP FUNCTION get_latest_signals_by_appsignalids(integer,text[]);

-- Drop SignalData type

DROP TYPE signaldata;


-- Create new SignalData type

CREATE TYPE public.signaldata AS
   (signalid integer,
    signalgroupid integer,
    signalinstanceid integer,
    changesetid integer,
    checkedout boolean,
    userid integer,
    channel integer,
    type integer,
    created timestamp with time zone,
    deleted boolean,
    instancecreated timestamp with time zone,
    action integer,
    appsignalid text,
    customappsignalid text,
    caption text,
    dataformatid integer,
    datasize integer,
    lowadc integer,
    highadc integer,
    lowengeneeringunits double precision,
    highengeneeringunits double precision,
    unitid integer,
    adjustment double precision,
    lowvalidrange double precision,
    highvalidrange double precision,
    unbalancelimit double precision,
    inputlowlimit double precision,
    inputhighlimit double precision,
    inputunitid integer,
    inputsensorid integer,
    outputlowlimit double precision,
    outputhighlimit double precision,
    outputunitid integer,
    outputsensorid integer,
    acquire boolean,
    calculated boolean,
    normalstate integer,
    decimalplaces integer,
    aperture double precision,
    inouttype integer,
    equipmentid text,
    outputrangemode integer,
    filteringtime double precision,
    spreadtolerance double precision,
    byteorder integer,
    enabletuning boolean,
    tuningdefaultvalue double precision,
    tuninglowbound double precision,
    tuninghighbound double precision,
    bustypeid text,
    adaptiveaperture boolean);*/
