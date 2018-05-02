-- Add new columns into signalInstance table

ALTER TABLE public.signalinstance ADD COLUMN specpropstruct text;
ALTER TABLE public.signalinstance ADD COLUMN specpropvalues bytea;
ALTER TABLE public.signalinstance ADD COLUMN protodata bytea;

