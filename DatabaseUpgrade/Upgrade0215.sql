-- Removed update of SignalPropertyBehavior.csv

SELECT 1;

-- Add new columns into signalInstance table

ALTER TABLE public.signalinstance ADD COLUMN specpropstruct text;
ALTER TABLE public.signalinstance ADD COLUMN specpropvalues bytea;
ALTER TABLE public.signalinstance ADD COLUMN protodata bytea;

